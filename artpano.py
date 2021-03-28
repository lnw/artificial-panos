#!/usr/bin/env python3.9

import os
import signal
import sys
import math
import subprocess
from argparse import ArgumentParser

sys.path.append('build')
import libartpano as ap
# import ap

deg2rad = math.pi/180.0
rad2deg = 180.0/math.pi

def parseCommandline():
    parser = ArgumentParser(prog="artpano.py", description="nothing yet", add_help=True)
    parser.add_argument("--lat", help="foo", dest="pos_lat", action="store", type=float, required=True)
    parser.add_argument("--lon", help=" ", dest="pos_lon", action="store", type=float, required=True)
    parser.add_argument("--ele", help=" ", dest="pos_ele", action="store", type=float, default=-1.0)
    parser.add_argument("--view-dir-h", help=" ", dest="view_dir_h", action="store", type=float, required=True)
    parser.add_argument("--view-dir-v", help=" ", dest="view_dir_v", action="store", type=float, default=0.0)
    parser.add_argument("--view-width", help=" ", dest="view_width", action="store", type=float, default=100.0)
    parser.add_argument("--view-height", help=" ", dest="view_height", action="store", type=float, default=0.0) # 20.0
    parser.add_argument("--range", help="visibility range in meters",
                        dest="range", action="store", type=float, default=10000.0)
    parser.add_argument("--canvas-width", help="width of the rendered picture in pixels",
                        dest="canvas_width", action="store", type=int, default="10000")
    parser.add_argument("--canvas-height", help="height of the rendered picture in pixels",
                        dest="canvas_height", action="store", type=int, default="0") # 2000
    parser.add_argument("--output", "-o", help="output filename",
                        dest="out_filename", action="store", type=str, default="out.png")
    parser.add_argument("--server", help="server from which elevation tiles are fetched",
                        dest="server", action="store", type=int, default=0)
    parser.add_argument("--source", help="source type and resolution",
                        dest="source", nargs='+', action="store", default=["view1","srtm1"]) # '+' meaning one or more arguments which end up in a list
    argparse = parser.parse_args()
    if argparse.view_height == 0.0 and argparse.canvas_height == 0:
        argparse.view_height = 20.0
        argparse.canvas_height = 2000
    elif argparse.view_height == 0.0:
        argparse.view_height = argparse.canvas_height * argparse.view_width / argparse.canvas_width
    elif argparse.canvas_height == 0:
        print(argparse.view_height)
        argparse.canvas_height = int(argparse.view_height * argparse.canvas_width / argparse.view_width)
    argparse.pos_lat *= deg2rad
    argparse.pos_lon *= deg2rad
    argparse.view_dir_h *= deg2rad
    argparse.view_dir_v *= deg2rad
    argparse.view_width *= deg2rad
    argparse.view_height *= deg2rad
    return argparse

def getElevationTiles(requiredTiles,sources):
    # from http://katze.tfiu.de/projects/phyghtmap/index.html
    folder = {'srtm1':'SRTM1v3.0', 'srtm3':'SRTM3v3.0', 'view1':'VIEW1', 'view3':'VIEW3'}
    for south,west in requiredTiles:
      for source in sources:
        # print(south, west)
        path = 'hgt/'+folder[source]+'/N{:02}E{:03}.hgt'.format(south,west)
        if (os.path.isfile(path)):
          print(path + " already exists")
          break
        else:
          subprocess.run(["phyghtmap", "--download-only", "--source={}".format(folder[source]), "-a {:03}:{:02}:{:03}:{:02}".format(west,south,west+1,south+1)])
          inpath = 'hgt/'+folder[source]+'/N{:02}E{:03}.tif'.format(south,west)
          outpath = 'hgt/'+folder[source]+'/N{:02}E{:03}.hgt'.format(south,west)
          if (os.path.isfile(outpath)): # loaded hgt
              break
          if (os.path.isfile(inpath)): # loaded geotif
              subprocess.run(["gdal_translate", "-ot", "UInt16", "-of", "SRTMHGT", "{}".format(inpath), "{}".format(outpath)])
              break
              #gdal_translate -ot UInt16 -of SRTMHGT N59E010.tif N59E010.hgt

def getOSMTiles(requiredTiles):
    # from github.com:mvexel/overpass-api-python-wrapper.git
    import overpass
    for south,west in requiredTiles:
      # print(west, south)
      path_peak = 'osm/N{:02}E{:03}_peak.osm'.format(south,west)
      path_coast = 'osm/N{:02}E{:03}_coast.osm'.format(south,west)
      path_isl = 'osm/N{:02}E{:03}_isl.osm'.format(south,west)
      if (os.path.isfile(path_peak)):
        print(path_peak + " already exists")
      else:
        print("downloading " + path_peak)
        api = overpass.API()
        query_peak = ('(' +
                      'node[natural=peak]({},{},{},{});'.format(south,west,south+1,west+1) +
                      ');')
        result_peak = api.Get(query_peak, responseformat='xml', verbosity='body')
        # print(query_peak)
        # print(result_peak)
        with open(path_peak, 'w') as f:
          f.write(result_peak)
      if (os.path.isfile(path_coast)):
        print(path_coast + " already exists")
      else:
        print("downloading " + path_coast)
        api = overpass.API()
        query_coast = ('(' +
                       'way[natural=water]({},{},{},{});(._;>;);'.format(south,west,south+1,west+1) +
                       'way[natural=coastline]({},{},{},{});(._;>;);'.format(south,west,south+1,west+1) +
                       #'relation[natural=water]({},{},{},{});(._;>;);'.format(south,west,south+1,west+1) +
                       ');')
        result_coast = api.Get(query_coast, responseformat='xml', verbosity='body')
        # print(result)
        with open(path_coast, 'w') as f:
          f.write(result_coast)
      if (os.path.isfile(path_isl)):
        print(path_isl + " already exists")
      else:
        print("downloading " + path_isl)
        api = overpass.API()
        query_isl = ('(' +
                     'node[place=island]({},{},{},{});(._;>;);'.format(south,west,south+1,west+1) +
                     'node[place=islet]({},{},{},{});(._;>;);'.format(south,west,south+1,west+1) +
                     #'relation[place=island]({},{},{},{});(._;>;);'.format(west,south,west+1,south+1) +
                     ');')
        result_isl = api.Get(query_isl, responseformat='xml', verbosity='body')
        # print(result_isl)
        with open(path_isl, 'w') as f:
          f.write(result_isl)

# def signal_handler(signal, frame):
#     print('You pressed Ctrl+C!')
#     sys.exit(0)

def main():
    # signal.signal(signal.SIGINT, signal_handler)
    args = parseCommandline()
    print(args)
    requiredTiles = ap.scene.determine_required_tiles(args.view_width, args.range, args.view_dir_h, args.pos_lat, args.pos_lon)
    print("required tiles: " + str(requiredTiles))
    getElevationTiles(requiredTiles,args.source)
    getOSMTiles(requiredTiles)
    # print('init S:')
    # print(args.source)
    S = ap.scene(args.pos_lat, args.pos_lon, args.pos_ele, args.view_dir_h, args.view_width, args.view_dir_v, args.view_height, args.range, args.source)
    # print(S)
    C = ap.canvas(args.out_filename, args.canvas_width, args.canvas_height)
    C.bucket_fill(100,100,100)
    C.render_scene(S)
    C.highlight_edges()
    C.construct_image()
#    C.draw_coast(S)
    C.annotate_peaks(S)
#    C.annotate_islands(S)
    C.label_axis(S)

if __name__ == "__main__":
    main()

