# MPTREE

It is a prototype of a multihoming proxy written in ANSI C89. The goal is to implement a multihoming protocol with TCP as transport protocol and a proxy system that act like SCTP o Multipath-TCP at application level. It was coded in 2008 as Computer Network exam and now it need an hard refactor. This repository may be a try to continue the project and to boost it as a net utility application for *nix. 

The program run in Linux,NetBSD,FreeBSD,OpenBSD and MAC OS X. The project may be a prototype for a research in the multihoming network.

## Current Status
Proxy_sender is a C language net utility that communicate with Proxy_receiver. An application should connect to Proxy_sender and send packet to it. Proxy_sender make a connect to Proxy_receiver and Proxy_receiver make a connect to the server of the client application. 
If Proxy_sender is connected to a multiple channel network the communication is multihomed and all connection channel are used for the client session. 

At the moment the communication from client to server is half-duplex because a better sending strategy is needed to finish this development phase. So if you want to test the proxy you need to write a dummy half-duplex TCP client. The numbers of channels supported is three and in the development phase a network simulator is used (see the following sections of this README).

## Congestion Control

## Send Strategy

## Receive Strategy


## Test
