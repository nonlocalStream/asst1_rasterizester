import math
import random
from colorsys import rgb_to_hsv, hsv_to_rgb
# <polygon fill="#1865ED" points="0,213.637 123.343,0 246.686,213.637 "/>
# <polygon fill="#18ED18" points="317.662,163.637 448.343,0 487.211,129.546 "/>
# <polygon fill="#EF161F" points="15.621,302.273 251.417,293.182 487.211,263.637 "/>
# <polygon fill="#EDB518" points="241.525,572.727 25.616,572.727 25.616,356.817 "/>
# <polygon fill="#ED18ED" points="464.781,631.819 478.417,309.091 471.599,642.045 "/>
# </svg>
def hex(rgb):
    return '%02x%02x%02x' % rgb

def hex_to_rgb(value):
   value = value.lstrip('#')
   lv = len(value)
   return tuple(int(value[i:i+lv/3], 16) for i in range(0, lv, lv/3))


def rgb_to_hex(rgb):
   return '%02x%02x%02x' % rgb


def polygon(points, fill="#ffffff", opacity=1):
    s = '\n<polygon fill="'+fill+'" fill-opacity="'+str(opacity)+'" points="'
    for (x,y) in points:
        s = s+str(x)+","+str(y)+" "
    return s+'"/>\n'

def transform(trans, svg):
    return '\n<g transform="'+trans+'">\n'+svg+"\n</g>\n"

def star(x, y, r, c="#cccc00"):
    k = 20
    p = 360.0/k
    points = []
    for i in range(20):
        deg = math.radians(p*i)
        points.append((math.cos(deg)*r, math.sin(deg)*r))
    return transform("translate("+str(x)+" "+str(y)+")", polygon(points, c))
    # return '<g transform="translate('+str(x)+" "+str(y)+')">' + polygon(points, "#cccc00")+ "</g>"

def cloud(X, Y, R, k, star_r, base_c, darkness=1):
    s = ""
    base_rgb = hex_to_rgb(base_c)
    hsv = rgb_to_hsv(base_rgb[0]/255.0,base_rgb[1]/255.0,base_rgb[2]/255.0)
    for i in range(k):
        r_ratio = abs(random.gauss(0,0.5))
        r = r_ratio*R
        rgb = hsv_to_rgb(min(1,hsv[0]+0.1*random.random()),hsv[1], 1-darkness*min(1,r_ratio)) 
        c = rgb_to_hex((rgb[0]*255, rgb[1]*255, rgb[2]*255))
        deg = random.random()*360
        x = math.cos(deg)*r
        y = math.sin(deg)*r
        s = s+star(X+x, Y+y, random.random()*star_r, c)
    return s

def spiral(spread, l, base_c):
    k = 30
    p = 360.0/k
    s = ""
    for i in range(k * l):
        r = i*spread
        deg = math.radians(p*i)
        # s = s + star(math.cos(deg)*r, math.sin(deg)*r,1)
        s = s + cloud(math.cos(deg)*r, math.sin(deg)*r,r/5, 10, 5, base_c, 0.5)
    return s

    
doc = """<?xml version="1.0" encoding="utf-8"?>
<!-- Generator: Adobe Illustrator 16.0.4, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
     width="500px" height="500px" viewBox="0 0 500 500" enable-background="new 0 0 500 500"
     xml:space="preserve">
"""

w = 500
h = 500
doc = doc + polygon([(0,0),(w,0),(w,h),(0,h)],"#000000", 1)
# doc = doc+transform("translate("+str(w/2)+" "+str(h/2)+")",star(10, 10, 10))+ "\n</svg>"
doc = doc+transform("translate("+str(w/2)+" "+str(h/2)+")",cloud(0,0,300,2000, 10, "#00cccc",1))
doc = doc+transform("translate("+str(w/2)+" "+str(h/2)+") scale(1 0.6)",spiral(4, 6, "#ffff00"))+"\n</svg>"
# doc = doc+transform("translate("+str(w/2)+" "+str(h/2)+") scale(-1 0.5)",spiral(4, 6, "ff99ff"))+ "\n</svg>"
f= open('star.svg','w')
f.write(doc)
f.close()