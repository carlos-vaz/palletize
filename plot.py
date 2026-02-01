import sys
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

boxes = []
palletX = 0
palletY = 0
minH = 0
maxH = 0

def parseFile(filename):
    global palletX
    global palletY
    global boxes
    global minH
    global maxH
    with open(filename, "r") as f:
        lines = [line.strip() for line in f]
    palletX = int(lines[0].split('=')[1].split(',')[0])
    palletY = int(lines[0].split('=')[1].split(',')[1])
    heights = []
    for li in lines[1:]:
        id = li.split()[0].split('=')[1]
        x = int(li.split()[1].split('=')[1].split(',')[0])
        y = int(li.split()[1].split('=')[1].split(',')[1])
        l = int(li.split()[2].split('=')[1].split(',')[0])
        w = int(li.split()[2].split('=')[1].split(',')[1])
        h = int(li.split()[2].split('=')[1].split(',')[2])
        boxes.append((id, x, y, l, w, h))
        heights.append(h)
    minH = min(heights)
    maxH = max(heights)

def heightToColor(h): 
    if(maxH==minH):
        return (0.5,0,0.5)
    r = (h-minH)/(maxH-minH)
    b = 1-r
    return (r,0,b)

def plot():
    fig, ax = plt.subplots()
    ax.set_xlim(0, palletX)
    ax.set_ylim(0, palletY)
    ax.set_aspect("equal", adjustable="box")
    ax.set_title("SKU Placements")

    for id, x, y, l, w, h in boxes:
        rect = Rectangle((x, y), l, w, facecolor=heightToColor(h), edgecolor="black", alpha=0.7)
        ax.add_patch(rect)
        ax.text(x+l/2, y+w/2, id, ha="center", va="center", fontsize=9, weight="bold")

    plt.show()

def main():
    parseFile(sys.argv[1])
    plot()

if(__name__=="__main__"):
    main()