# bPulse

`bPulse` is an [iPulse](https://ipulseapp.com)-inspired graphical system monitoring tool for `Linux` and `MacOS`. It is written in `C++` and relies on `X11`, `X11's Shape Extension`, `X11's Back-buffer Extension`, `Xrender`, `libpng`, and `FreeType` fonts. `bPulse` is capable of monitoring CPU, memory, disk-I/O, network-I/O, disk usage, users logged in, and keep track of the date and time.

Running `bPulse` on `MaOS` requires that the [XQuartz](https://www.xquartz.org) `X11` window manager is installed.

## Usage

`bPulse`  is compiled with:

```shell
make
```

This results in a binary executable called `bpulse`, which can be invoked as follows:

```shell
./bpulse &
```

## Theming

`bPulse` uses a straight-forward theming system that relies on a simple text (`.theme`) file and PNG images. The default theme located in the  [data](data/)-directory, can be the starting point for one's own creations.

## Notes

1. On `MacOS` XCode and the developer tools needs to be installed.
2. Both `Linux` and `MacOS` require `libpng`.

## BSD-3 License

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
