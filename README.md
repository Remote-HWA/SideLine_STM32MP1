# SideLine - How Delay-Lines (May) Leak Secrets from your SoC

## About SideLine

SideLine is a novel side-channel vector based on delay-line components widely implemented in high-end SoCs. In the associated paper, we provide a detailed method on how to access and convert delay-line data into power consumption information and we demonstrate that these entities can be used to perform remote power side-channel attacks. We report experiments carried out on two SoCs from distinct vendors and we recount several core-vs-core attack scenarios in which an adversary process located in one processor core aims at eavesdropping the activity of a victim process located in another core. For each scenario, we demonstrate the adversary ability to fully recover the secret key of an OpenSSL AES running in the victim core. Even more detrimental, we show that these attacks are still practicable if the victim or the attacker program runs over an operating system.

<p align="center">
<img src="https://user-images.githubusercontent.com/67143135/85726797-bac67600-b6f6-11ea-9162-8daf8975c3bd.png" width="700" height="250">
</p>
<p align="center"> Figure 1: The three attack scenarios presented in SideLine<p align="center">
  
## Content

This repository contains:
Source codes to reproduce the SideLine attack on STM32MP1 


## Requirements
- A STMicroelectronics STM32MP1 based development board.
- Working installations of STM32Cube IDE.










