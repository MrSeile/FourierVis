from svgpathtools import svg2paths
import numpy as np
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("path", type=str)
parser.add_argument("step", type=float)

args = parser.parse_args()

# path, atr = svg2paths("C:/Users/elies/Desktop/img.svg")

path, atr = svg2paths(args.path)

xs = []
ys = []

height = 55

step = args.step
for x in np.arange(0, 1, step):
	
	p = path[0].point(x)
	
	print(p.real / 1)
	print((height - p.imag) / 1)
