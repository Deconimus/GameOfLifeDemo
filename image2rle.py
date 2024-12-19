import os, sys, argparse
import PIL.Image, PIL.ImageOps
import numpy as np


def main(args):
    if not args:
        return
    
    for file in args.files:
        img = PIL.Image.open(file)
        w, h = img.size
        
        if w <= 0 or h <= 0:
            print("Cannot read \""+file+"\".")
            continue
        
        img = PIL.ImageOps.grayscale(img)
        mat = np.asarray(img, dtype=np.uint8)
        
        if args.invert:
            mat = (mat < 0.5).astype(bool)
        else:
            mat = (mat >= 0.5).astype(bool)
        
        outfile = file[:file.rindex(".")]+".rle"
        
        output  = "# "+("-"*78)+"\n"
        output += "# RLE file created by image2rle.py (by deconimus)\n"
        output += "# Original file: "+file+"\n"
        output += "# "+("-"*78)+"\n"
        output += "x = "+str(w)+", y = "+str(h)+"\n"
        
        for y in range(h):
            row = ""
            current = mat[y,0]
            count = 1
            
            for x in range(1,w):
                val = mat[y,x]
                if val == current:
                    count += 1
                else:
                    row += (str(count) if count > 1 else "") + ("o" if current else "b")
                    count = 1
                    current = val
            
            if current:
                row += (str(count) if count > 1 else "") + "o"
            row += "$"
                
            output += row
            
        output += "!\n"
        
        with open(outfile, "w+") as f:
            f.write(output)
    
        print(outfile)
    
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="image2rle", description="A script to convert image files to Game of Life RLE files.")
    parser.add_argument("files", nargs="+")
    parser.add_argument("-i", "--invert", action='store_true', default=False) # use black pixels instead of white as ALIVE

    main(parser.parse_args())
    