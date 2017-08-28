#!/usr/bin/env python3

import os
import signal
import sys
import math
from argparse import ArgumentParser

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
    parser.add_argument("--view-height", help=" ", dest="view_height", action="store", type=float, default=20.0)
    parser.add_argument("--range", help="visibility range in meters",
                        dest="range", action="store", type=float, default=10000.0)
    parser.add_argument("--canvas-width", help="width of the rendered picture in pixels",
                        dest="canvas_width", action="store", type=int, default="10000")
    parser.add_argument("--canvas-height", help="height of the rendered picture in pixels",
                        dest="canvas_height", action="store", type=int, default="1000")
    parser.add_argument("--output", "-o", help="output filename",
                        dest="out_filename", action="store", type=str, default="out.png")
    parser.add_argument("--server", help="server from which elevation tiles are fetched",
                        dest="server", action="store", type=int, default=0)
    parser.add_argument("--source", help="source type and resolution",
                        dest="source", nargs='+', action="store", default=["srtm1","view1"]) # '+' meaning one or more arguments which end up in a list
    ap = parser.parse_args()
    ap.pos_lat *= deg2rad
    ap.pos_lon *= deg2rad
    ap.view_dir_h *= deg2rad
    ap.view_dir_v *= deg2rad
    ap.view_width *= deg2rad
    ap.view_height *= deg2rad
    return ap

def getElevationTiles(requiredTiles):
    for west,south in requiredTiles:
        # print(west, south)
        path = 'hgt/N{:02}E{:03}.hgt'.format(west,south)
        if (os.path.isfile(path)):
            print(path + " already exists")
        else:
            print("should download " + path + " (not implemented yet)")

def getOSMTiles(requiredTiles):
    # from github.com:mvexel/overpass-api-python-wrapper.git
    import overpass
    for west,south in requiredTiles:
        # print(west, south)
        path = 'osm/N{:02}E{:03}.osm'.format(west,south)
        if (os.path.isfile(path)):
            print(path + " already exists")
        else:
            print("downloading " + path)
            api = overpass.API()
            query = ('(' +
                    'node[natural=peak]({},{},{},{});'.format(west,south,west+1,south+1) +
                    'way[natural=water]({},{},{},{});(._;>;);'.format(west,south,west+1,south+1) +
                    'way[natural=coastline]({},{},{},{});(._;>;);'.format(west,south,west+1,south+1) +
                    'relation[natural=water]({},{},{},{});(._;>;);'.format(west,south,west+1,south+1) +
                    ');')
            result = api.Get(query, responseformat='xml', verbosity='body')
            # water bodies
            # print(result)
            with open(path, 'w') as f:
                f.write(result)

# def signal_handler(signal, frame):
#     print('You pressed Ctrl+C!')
#     sys.exit(0)

def main():
    # signal.signal(signal.SIGINT, signal_handler)
    args = parseCommandline()
    print(args)
    requiredTiles = ap.scene.determine_required_tiles(args.view_width, args.range, args.view_dir_h, args.pos_lat, args.pos_lon)
    print("required tiles: " + str(requiredTiles))
    getElevationTiles(requiredTiles)
    getOSMTiles(requiredTiles)
    # print('init S:')
    # print(args.source)
    S = ap.scene(args.pos_lat, args.pos_lon, args.pos_ele, args.view_dir_h, args.view_width, args.view_dir_v, args.view_height, args.range, args.source)
    # print(S)
    C = ap.canvas(args.out_filename, args.canvas_width, args.canvas_height)
    C.bucket_fill(100,100,100)
    C.render_scene(S)
    C.highlight_edges()
    C.annotate_peaks(S)
    C.label_axis(S)

if __name__ == "__main__":
    main()

