# bPulse

`bPulse` is an [iPulse](https://ipulseapp.com)-inspired (by
[iconfactory](https://iconfactory.com)) graphical system monitoring tool for
`Linux` and `MacOS`. It is written in `C++` and can run directly under `X11` or
through [`GLFW`](https://glfw.org). Running directly under `X11`, either `GLX`
or `Xrender` can be selected as the drawing backend and  relies on 
`X11's Shape Extension` and `X11's Back-buffer Extension`. `bPulse` further
relies on `libpng` and [`FreeType`](https://freetype.org) for font loading and
 rendering. `bPulse` monitors CPU, memory, disk-I/O, network-I/O, disk usage,
 users logged in, and keep track of the date and time.

Running `bPulse` directly under `X11` on `MaOS` requires that
[XQuartz](https://www.xquartz.org) installed. Running under `GLFW` has no such
requirement.

![bPulse in Action](bPulse.png "bPulse in Action")

## Usage

`bPulse`  is compiled with:

```shell
make USE_XRender=1
```

to use the `Xrender` backend,

```shell
make USE_GLX=1
```

to use the `GLX` backend for drawing graphics, or

```shell
make USE_GLFW=1
```

to use the `GLFW` backend for drawing graphics.

This results in a binary executable called `bpulse`, which can be invoked as follows:

```shell
./bpulse &
```

## Theming

`bPulse` uses a straight-forward theming system that relies on a simple text (`.theme`) file and PNG images. The default theme located in the  [data](data/)-directory, can be the starting point for one's own creations.

## Notes

1. On `MacOS` XCode and the developer tools needs to be installed.
2. Both `Linux` and `MacOS` require `libpng` and `FreeType`.
3. Running under `GLFW` requires version 3.4 or higher to be installed.

## BSD-3 License

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
