# yuugen

yuugen is a WIP Nintendo DS emulator written in C++

## currently yuugen is in the process of a rewrite and has the current goals in mind:
- refactor arm core and add a jit with a64 and x64 backends, alongside a common IR (Intermediate Representation) to allow for common optimisations between host architectures
- provide abstractions for different audio and video backends
- create a custom ui framework which is rendered via a video backend, with inspiration from the style of the ps3 xmb menu
- create debugging facilities to further aid improving compatibility
- try to make everything generic where possible. e.g. separate processors from systems