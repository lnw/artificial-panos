#!/usr/bin/env python

import os
import signal
import sys
import math
import subprocess
from argparse import ArgumentParser

sys.path.append('build')
import libartpano as ap

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
    parser.add_argument("--range", help="visibility range [km]",
                        dest="range_km", action="store", type=float, default=20.0)
    parser.add_argument("--canvas-width", help="width of the rendered picture in pixels",
                        dest="canvas_width", action="store", type=int, default="10000")
    parser.add_argument("--canvas-height", help="height of the rendered picture in pixels",
                        dest="canvas_height", action="store", type=int, default="0") # 2000
    parser.add_argument("--output", "-o", help="output filename",
                        dest="out_filename", action="store", type=str, default="out.png")
    parser.add_argument("--server", help="server from which elevation tiles are fetched",
                        dest="server", action="store", type=int, default=0)
    parser.add_argument("--source", help="source type and resolution",
                        dest="source", nargs='+', action="store", default=["view1","srtm1","view3","srtm3"]) # '+' meaning one or more arguments which end up in a list
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

def getElevationTiles(requiredTiles, sources):
    # from http://katze.tfiu.de/projects/phyghtmap/index.html
    folder = {'srtm1':'SRTM1v3.0', 'srtm3':'SRTM3v3.0', 'view1':'VIEW1', 'view3':'VIEW3'}
    for south, west in requiredTiles:
      for source in sources:
        # print(south, west)
        coordstring = '{}{:02}{}{:03}'.format('N' if south >= 0 else 'S',abs(south),'E' if west >=0 else 'W',abs(west))
        path = 'hgt/' + folder[source] + '/' + coordstring + '.hgt'
        if (os.path.isfile(path)):
          print(path + " already exists")
          break
        else:
          subprocess.run(["phyghtmap", "--earthexplorer-user=lnwz", "--earthexplorer-password=f73x8qGFzmwT", "--download-only", "--source={}".format(folder[source]), "-a {:03}:{:02}:{:03}:{:02}".format(west,south,west+1,south+1)])
          inpath = 'hgt/' + folder[source] + '/' + coordstring + '.tif'
          outpath = 'hgt/' + folder[source] + '/' + coordstring + '.hgt'
          if (os.path.isfile(outpath)): # loaded hgt
              break
          if (os.path.isfile(inpath)): # loaded geotif
              subprocess.run(["gdal_translate", "-ot", "UInt16", "-of", "SRTMHGT", "{}".format(inpath), "{}".format(outpath)])
              break
              #gdal_translate -ot UInt16 -of SRTMHGT N59E010.tif N59E010.hgt

def getOSMTiles(requiredTiles):
    # from github.com:mvexel/overpass-api-python-wrapper.git
    import overpass
    for north, east in requiredTiles: # which are the south-west corners of respective tiles
      # print(west, south)
      path_peak = 'osm/{}{:02}{}{:03}_peak.osm'.format('N' if north>=0 else 'S', abs(north), 'E' if east>=0 else 'W', abs(east))
      path_coast = 'osm/{}{:02}{}{:03}_coast.osm'.format('N' if north>=0 else 'S', abs(north), 'E' if east>=0 else 'W', abs(east))
      path_isl = 'osm/{}{:02}{}{:03}_isl.osm'.format('N' if north>=0 else 'S', abs(north), 'E' if east>=0 else 'W', abs(east))
      if (os.path.isfile(path_peak)):
        print(path_peak + " already exists")
      else:
        print("downloading " + path_peak)
        api = overpass.API()
        query_peak = ('(' +
                      'node[natural=peak]({},{},{},{});'.format(north,east,north+1,east+1) +
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
                       'way[natural=water]({},{},{},{});(._;>;);'.format(north,east,north+1,east+1) +
                       'way[natural=coastline]({},{},{},{});(._;>;);'.format(north,east,north+1,east+1) +
                       #'relation[natural=water]({},{},{},{});(._;>;);'.format(north,east,north+1,east+1) +
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
                     'node[place=island]({},{},{},{});(._;>;);'.format(north,east,north+1,east+1) +
                     'node[place=islet]({},{},{},{});(._;>;);'.format(north,east,north+1,east+1) +
                     #'relation[place=island]({},{},{},{});(._;>;);'.format(east,north,east+1,north+1) +
                     ');')
        result_isl = api.Get(query_isl, responseformat='xml', verbosity='body')
        # print(result_isl)
        with open(path_isl, 'w') as f:
          f.write(result_isl)

# def signal_handler(signal, frame):
#     print('You pressed Ctrl+C!')
#     sys.exit(0)

def strings2enums(sources):
    res = []
    if "srtm1" in sources:
        res.append(ap.elevation_source.srtm1)
    if "srtm3" in sources:
        res.append(ap.elevation_source.srtm3)
    if "view1" in sources:
        res.append(ap.elevation_source.view1)
    if "view3" in sources:
        res.append(ap.elevation_source.view3)
    return res

def main():
    # signal.signal(signal.SIGINT, signal_handler)
    args = parseCommandline()
    print(args)
    pos = ap.latlonfp(args.pos_lat, args.pos_lon)
    requiredTiles_ll = ap.scene.determine_required_tiles(args.view_width, 1000 * args.range_km, args.view_dir_h, pos)
    requiredTiles = ap.vll2vp_int64(requiredTiles_ll)
    print("required tiles: " + str(requiredTiles))
    getElevationTiles(requiredTiles, args.source)
    getOSMTiles(requiredTiles)
    # print('init S:')
    # print(args.source)
    S = ap.scene(pos, args.pos_ele, args.view_dir_h, args.view_width, args.view_dir_v, args.view_height, 1000 * args.range_km, strings2enums(args.source))
    # print(S)
    C = ap.canvas_t(args.canvas_width, args.canvas_height)
    C.bucket_fill(100,100,100)
    C.render_scene(S)
    C.highlight_edges()

    CC = ap.canvas(args.out_filename, C)
#    CC.draw_coast(S)
    CC.annotate_peaks(S)
#    CC.annotate_islands(S)
    CC.label_axis(S)
    CC.write_png()

if __name__ == "__main__":
    main()

