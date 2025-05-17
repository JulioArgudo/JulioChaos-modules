# JulioChaos-modules
A [VCV Rack](https://vcvrack.com/) Plugin for Chaotic Oscillator modules

This plugin includes the module Chua's Oscillator, based on the chaotic behaviour of [Chua's circuit](https://www.chuacircuits.com/diagram.php)

<img width="225" alt="Chua panel screenshot" src="https://github.com/user-attachments/assets/20a69a28-0ef2-4a42-8b7d-f83d1a1e3654" /> <br/>

<img width="264" alt="Chua osciloscope 1" src="https://github.com/user-attachments/assets/605b9070-16a8-4cb6-9c8b-9cc936b8abab" />
<img width="251" alt="Chua osciloscope 2" src="https://github.com/user-attachments/assets/68e8f63a-ef84-4678-a8f0-923f4d5ffdfc" />
<img width="272" alt="Chua osciloscope 3" src="https://github.com/user-attachments/assets/7d3d71d9-3ae8-4af6-90a9-913dfb1e922a" />

## Chua's ecuations

The Chua's Oscillator module follows the following differential ecuations modeling Chua's circuit:

> f(x) = m1\*x+(m0-m1)/2\*(|x+1|-|x-1|) <br/>
> dx/dt = c1\*(y-x-f(x)) <br/>
> dy/dt = c2\*(x-y+z) <br/>
> dz/dt = -c3\*y

These ecuations are solved using [Euler's method](https://en.wikipedia.org/wiki/Euler_method), aproximating each increment with a straight line, and scaling the increment by sample with the parameter **Rate**. This parameter has an effect similar to a "frecuency" or "pitch" parameter in periodic oscillators. The button **HF/LF** changes the range of the **Rate** knob and CV modulation input between high frecuency and low frecuency.

The other three parameters **P1**, **P2** and **P3**, are mapped to the values of the ecuations constans m0, m1, c1, c2 and c3. These have unpredictable effects on the generated signal due to its chaotic nature.

Since the ecuations are 3-dimensional, each point has 3 independent components, each mapped to one of the three outputs **X**, **Y** and **Z**.

There is also a **Reset** input, which receives Trigger signals and returns the circuit to the initial conditions. Since the circuit is deterministic, this also serves as a Sync input.


## Signals

Chua's circuit is a chaotic non-linear system, known as a [strange attractor](https://en.m.wikipedia.org/wiki/Attractor#Strange_attractor), which means the signals generated are deterministic but non-periodic. Depending on the values of its parameters, the system has periodic states, noise-like states, and a spectrum of in-between states.


<img width="1434" alt="Chua spec noise" src="https://github.com/user-attachments/assets/8556cc73-d31f-4abf-b14b-231a98b45e7e" /><br/>
<img width="1433" alt="Chua spec harm" src="https://github.com/user-attachments/assets/ce758b03-37a2-4d15-bb00-e0f5b39d7506" />


The three signal outputs **X**, **Y** and **Z**, corresponding to the three components of the system, have similar behaviours but different spectral qualities, which can be useful when used as an audio signal.

<img width="1433" alt="Chua spec XYZ" src="https://github.com/user-attachments/assets/7a51c7c9-dd60-4442-89cb-03e592c5229e" />

