<div align="center">
<h3 align="center">Computer Networks</h3>

  <a href=""> ![Static Badge](https://img.shields.io/badge/Language-Java-6866fb)</a>
  <a href=""> ![Static Badge](https://img.shields.io/badge/Semester-6-green)</a>
</div>

This repository contains the project that I completed for the Computer Networks I 2022-2023 Course, as part of my undergraduate studies at Aristotle University of Thessaloniki.

|   Course Information     |                                  |
|--------------------------|----------------------------------|
| Semester                 | 6                                |
| Attendance Year          | 2022-2023                        |
| Project Type             | Optional                         |
| Team Project             | No                               |
| Language Used            | Java                             |
| Software Used            | Eclipse                          |

## Description

As part of the Computer Networks 1 course, we were asked to create a virtual modem in Java, that would communicate with the professor's "Ithaki" server. Our virtual modem should be able to do the following:
* Connect to the ithaki server and initialize serial communication
* Request and receive echo packets from the server
* Request, receive and save images from the server
* Request and receive GPS traces from the server
* Communicate with the server using the server's ARQ protocol

## Data Collection and Results Presentation
As part of this project, we were asked to present results from at least two sessions with the virtual lab server, spaced at least 48 hours apart.
For each session we were asked to include in our report:
  - At least one graph displaying response time in milliseconds for each packet sent during a minimum 4-minute interval.
  - Two images received from the server, one without transmission errors and one with errors.
  - At least one image with at least four GPS traces, each separated by at least four seconds.
  - Another graph displaying response time in milliseconds for each successfully received packet using ARQ in conditions of pseudo-random errors.
  - An estimation of the probability distribution for the number of retransmissions.
  - An estimation of the Error Rate and Bit Error Rate (BER).
    
### Report Requirements
The report should include the aforementioned results, accompanied by:
  - Brief comments or observations on the measurements and charts presented.
  - A small bibliographic technical reference to mechanisms and protocols used in similar modem applications.