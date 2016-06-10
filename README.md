
# gif2bmp - convert 8bit static GIF image to bitmap

## Installation
Just use make to compile all the sources.

```
$ make
```

## Usage

### Convert GIF image to BMP
```
./gif2bmp -i tests/input/jobs.gif -o jobs.bmp
```

### Print internal information of a GIF image
```
./gif2bmp -i tests/input/1x1.gif -p
```
### Convert GIF image to rgb colors
```
./gif2bmp -i tests/input/1.gif -o out.rgb -r 
```

### Print help for more information
```
./gif2bmp -h
```

## Documentation
Czech documentation can be found [here](https://github.com/mmsrubar/gif2bmp/blob/master/doc/gif2bmp.pdf).
