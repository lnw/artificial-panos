#!/usr/bin/env python3

import os
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
    parser.add_argument("--range", help=" ", dest="range", action="store", type=float, default=10000.0)
    parser.add_argument("--canvas-width", help=" ", dest="canvas_width", action="store", type=int, default="5000")
    parser.add_argument("--canvas-height", help=" ", dest="canvas_height", action="store", type=int, default="1000")
    parser.add_argument("--output", "-o", help=" ", dest="out_filename", action="store", default="out.png")
    ap = parser.parse_args()
    ap.pos_lat *= deg2rad
    ap.pos_lon *= deg2rad
    ap.view_dir_h *= deg2rad
    ap.view_dir_v *= deg2rad
    ap.view_width *= deg2rad
    ap.view_height *= deg2rad
    return ap


#def getElevationTiles(required_tiles):


#def getOSMTiles(required_tiles):


def main():
    args = parseCommandline()
    print(args)
    print('py: required tiles?')
    required_tiles = ap.scene.determine_required_tiles(args.view_width, args.range, args.view_dir_h, args.pos_lat, args.pos_lon)
    print(required_tiles)
    #getElevationTiles(required_tiles)
    #getOSMTiles(required_tiles)
    print('init S:')
    S = ap.scene(args.pos_lat, args.pos_lon, args.pos_ele, args.view_dir_h, args.view_width, args.view_dir_v, args.view_height, args.range)
    print(S)
    C = ap.canvas(args.out_filename, args.canvas_width, args.canvas_height)
    C.bucket_fill(100,100,100)
    C.render_scene(S)
    C.highlight_edges()
    C.annotate_peaks(S)
    C.label_axis(S)

if __name__ == "__main__":
    main()

