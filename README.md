# JulioChaos-modules
A [VCV Rack](https://vcvrack.com/) Plugin for Chaotic Oscillator modules

<img width="898" alt="Modules" src="https://github.com/user-attachments/assets/5c9c1239-e5c1-4b8a-9358-798100216be4" /><br/>

These modules solve the differential ecuations of each system using [Euler's method](https://en.wikipedia.org/wiki/Euler_method), aproximating each increment with a straight line, and scaling the increment by sample with the parameter **Rate**. This parameter has an effect similar to a "frecuency" or "pitch" parameter in periodic oscillators. The button **HF/LF** changes the range of the **Rate** knob and CV modulation input between high frecuency and low frecuency.

The other three parameters **P1**, **P2** and **P3**, are mapped to the values of the ecuations constans. These have unpredictable effects on the generated signal due to its chaotic nature.

Since the ecuations are 3-dimensional, each point has 3 independent components, each mapped to one of the three outputs **X**, **Y** and **Z**.

There is also a **Reset** input, which receives Trigger signals and returns the system to its initial conditions. Since the systems are deterministic, this also serves as a Sync input.


## Chua's oscillator

<img width="264" alt="Chua osciloscope 1" src="https://github.com/user-attachments/assets/605b9070-16a8-4cb6-9c8b-9cc936b8abab" />
<img width="251" alt="Chua osciloscope 2" src="https://github.com/user-attachments/assets/68e8f63a-ef84-4678-a8f0-923f4d5ffdfc" />
<img width="272" alt="Chua osciloscope 3" src="https://github.com/user-attachments/assets/7d3d71d9-3ae8-4af6-90a9-913dfb1e922a" />

The Chua's Oscillator module follows the following differential ecuations modeling [Chua's circuit](https://www.chuacircuits.com/diagram.php):

<img width="433" alt="Chua equations" src="https://github.com/user-attachments/assets/768b2efe-cf0f-44e2-b312-aa3cf4aed3c7" />

## Rossler's oscillator

The Rossler's oscillator module follows the following differential ecuations modeling [Rossler's attractor](https://en.wikipedia.org/wiki/R%C3%B6ssler_attractor):

<img width="255" alt="Rossler equations" src="https://github.com/user-attachments/assets/7cf31ceb-7e52-437a-9591-9e4eca26183b" />

## Lorentz84 oscillator

The Lorentz84 oscillator module follows the following differential ecuations modeling the [Lorentz84 system](https://docs.dart.ucar.edu/en/latest/models/lorenz_84/readme.html):

<img width="355" alt="Lorentz84 equations" src="https://github.com/user-attachments/assets/e39449e0-7a47-4fb0-b756-df1bb9e14c8b" />

## Signals

Chua's circuit is a chaotic non-linear system, known as a [strange attractor](https://en.m.wikipedia.org/wiki/Attractor#Strange_attractor), which means the signals generated are deterministic but non-periodic. Depending on the values of its parameters, the system has periodic states, noise-like states, and a spectrum of in-between states.


<img width="1434" alt="Chua spec noise" src="https://github.com/user-attachments/assets/8556cc73-d31f-4abf-b14b-231a98b45e7e" /><br/>
<img width="1433" alt="Chua spec harm" src="https://github.com/user-attachments/assets/ce758b03-37a2-4d15-bb00-e0f5b39d7506" />


The three signal outputs **X**, **Y** and **Z**, corresponding to the three components of the system, have similar behaviours but different spectral qualities, which can be useful when used as an audio signal.

<img width="1433" alt="Chua spec XYZ" src="https://github.com/user-attachments/assets/7a51c7c9-dd60-4442-89cb-03e592c5229e" />

