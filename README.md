# CX16-KAKURO

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ifilot/cx16-kakuro?label=version)
[![build](https://github.com/ifilot/cx16-kakuro/actions/workflows/build.yml/badge.svg)](https://github.com/ifilot/cx16-kakuro/actions/workflows/build.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

[Download latest version](https://github.com/ifilot/cx16-KAKURO/releases/latest/download/CX16-KAKURO.ZIP)

## Compilation

First, install the required dependencies

```bash
sudo apt-get install -y build-essential cc65 python3 python3-numpy python3-pilkit
```
Compilation is fairly straightforward. Go to the `src` folder and run `make`.

```bash
cd src && make
```

## Dependencies

CX16-KAKURO makes use of [zsmkit](https://github.com/mooinglemur/zsmkit) which
is an advanced music and sound effects engine for the Commander X16 and
available under a MIT License. A static copy of this library is bundled in
this repository and automatically embedded in the `.PRG` file.

## Assets

* All graphical artwork was created by me using [Pyxel
  Edit](https://pyxeledit.com/). No permission is required to use this artwork
  in your work, although attribution is always greatly appreciated.
* Background music "Corridors of Time" by GTR3QQ is available under a
  CC-BY-SA-4.0 license and obtained from [this source](https://github.com/mooinglemur/melodius/blob/main/ROOT/ZSM/Furnace/GTR3QQ/Corridors%20of%20Time.zsm)
  GTR3QQ is graciously acknowledged for producing this piece of music and offering
  it under a liberal license.

## Community guidelines

* Contributions to CX16-Othello are always welcome and appreciated. Before doing so,
  please first read the [CONTRIBUTING](CONTRIBUTING.md) guide.
* For reporting issues or problems with the software, you are kindly invited to
  to open a [new issue with the bug label](https://github.com/ifilot/cx16-othello/issues/new?labels=bug).
* If you seek support in using CX16-Othello, please 
  [open an issue with the question](https://github.com/ifilot/cx16-othello/issues/new?labels=question)
  label.
* If you wish to contact the developers, please send an e-mail to ivo@ivofilot.nl.

## License

Unless otherwise stated, all code in this repository is provided under the GNU
General Public License version 3.