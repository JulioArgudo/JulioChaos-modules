# JulioChaos-modules
A VCV Rack Plugin for Chaotic Oscillator modules

This plugin includes the module Chua's Oscillator, based on the chaotic behaviour of Chua's circuit

<img width="196" alt="Chua panel screenshot" src="https://github.com/user-attachments/assets/7073683f-dac3-4c9d-9254-928a8755642f" /> <br/>

<img width="264" alt="Chua osciloscope 1" src="https://github.com/user-attachments/assets/605b9070-16a8-4cb6-9c8b-9cc936b8abab" />
<img width="251" alt="Chua osciloscope 2" src="https://github.com/user-attachments/assets/68e8f63a-ef84-4678-a8f0-923f4d5ffdfc" />
<img width="272" alt="Chua osciloscope 3" src="https://github.com/user-attachments/assets/7d3d71d9-3ae8-4af6-90a9-913dfb1e922a" />

## Ecuations

The Chua's Oscillator module follows the following differential ecuations:

> f(x) = m1·x+(m0-m1)/2·(|x+1|-|x-1|) <br/>
> dx/dt = c1·(y-x-f(x)) <br/>
> dy/dt = c2·(x-y+z) <br/>
> dz/dt = -c3·y

These ecuations are solved using Euler's method, aproximating each increment with a straight line, and scaling the increment by sample with the parameter **Rate**. This parameter has an effect similar to a "frecuency" or "pitch" parameter in periodic oscillators. The button **HF/LF** changes the range of the **Rate** knob and CV modulation input between high frecuency and low frecuency.

The other three knobs P1, P2 and P3, are mapped to the values of the ecuations constans **m0**, **m1**, **c1**, **c2** and **c3**. These parameters have unpredictable effects on the generated signal due to its chaotic nature.

Since the ecuations are 3-dimensional, each point has 3 independent components, each mapped to one of the three outputs **X**, **Y** and **Z**.
