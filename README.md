<center><img src="https://github.com/pablomc88/COREM.github.ugr/blob/master/img/corem_logo_6_path.png" alt="COREM logo" height="80" width="340"></center>

COREM is a configurable simulator for retina modeling that has been implemented within the framework of the Human Brain Project (HBP). The software platform can be interfaced with neural simulators (e.g., NEST) to connect with models of higher visual areas and with the Neurorobotics Platform of the HBP. The code is implemented in C++ and computations of spatiotemporal equations are optimized by means of recursive filtering techniques and multithreading.

Most retina simulators are more focused on fitting specific retina functions. By contrast, the versatility of COREM allows the configuration of different retina models using a set of basic retina computational primitives. We implemented a series of retina models by combining these primitives to characterize some of the best-known phenomena observed in the retina: adaptation to the mean light intensity and temporal contrast, and differential motion sensitivity.

The code has been extensively tested in Linux. The software can be also adapted to Mac OS. Installation instructions as well as the user manual can be found in the [wiki](https://github.com/pablomc88/COREM/wiki). COREM is the result of a research work and its associated publication is:

**Martínez-Cañada, P., Morillas, C., Pino, B., Ros, E., & Pelayo, F. (2016). A Computational Framework for Realistic Retina Modeling. International Journal of Neural Systems, 26(07), 1650030.**

COREM is licensed under GNU General Public License. See [LICENSE](LICENSE) for more information. Some components of the source code are also licensed under CeCILL-C. 
