# COREM
COREM is a configurable simulator for retina modeling that has been implemented within the framework of the Human Brain Project (HBP). The software platform can be interfaced with neural simulators (e.g., NEST) to connect with models of higher visual areas and with the Neurorobotics Platform of the HBP. The code is implemented in C++ and computations of spatiotemporal equations are optimized by means of recursive filtering techniques and multithreading.

Most of the retina simulators are more focused on fitting specific retina functions. By contrast, the versatility of COREM allows the configuration of different retina models using a set of basic retina computational primitives. We implemented a series of retina models by combining these primitives to characterize some of the best-known phenomena observed in the retina: adaptation to the mean light intensity and temporal contrast, and differential motion sensitivity.

The code has been extensively tested in Linux. The software can be also installed in Mac OS with some minor modifications of the Makefile. Installation instructions as well as the user manual can be found in the [wiki](https://github.com/pablomc88/COREM/wiki). COREM is the result of a research work and its associated publication is:

`Martínez-Cañada, P., Morillas, C., Pino, B., Ros, E., Pelayo, F. A Computational Framework for Realistic Retina Modeling. In International Journal of Neural Systems. Accepted for publication.`

COREM is licensed under GNU General Public License. See [LICENSE](LICENSE) for more information. Some components of the source code are also licensed under CeCILL-C. 
