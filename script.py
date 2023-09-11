"""
This simple Python script renames all files in a folder to the naming 
convention adopted by the DFMini Player library for the Arduino Uno 
"""

import os

directory = 'test/'

def main():
    counter = 0
    for filename in os.listdir(directory):
        newname = directory + f"000{counter}.mp3"
        realfilename = directory + filename
        os.rename(realfilename, realfilename + newname)
        counter = counter + 1

if __name__ == '__main__':
    main()
