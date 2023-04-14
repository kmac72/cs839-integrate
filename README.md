# cs839-integrate

Our Integrate project is a wearable device that tracks cycling cadence and dynamically controls the cyclist's Spotify to match song BPM to the rider's cadence. 

## Installation

After cloning, open `cadence_bpm_system.ino` in the Arduino IDE. 

## Usage

## Current tasks

| Task                  | Status     |
| --------------------- | ---------- |
| :bug: Template Example Bug | In progress |
| :rocket: Add feature X | Not started|
| :biking_man: Potential Feature | If we have time|
|--- |--- |
| 🚀 Connect ESP to Moofit Sensor | ✔️ Done|
| 🚀 Search Spotify for songs by BPM | ✔️ Done |
| 🚀 Play a given Spotify song | ✔️ Done|
| 🚀 Spin up EC2 Instance | ✔️ Done |
| 🚀 Serial out cadence data | ✔️ Done |
| :biking_man: Stop playback if effort wanes to encourage effort | Potential |
| 🚀 Calculate cadence moving average | In progress |
| 🚀 Feed cadence avg to Spotify functionality (put everything together!) | Not started |


 



## Troubleshooting
<ul>
 <li> If you're on MacOS getting weird Serial output, make sure your Serial is rendering at 115200 baud. </li>
 <li> If you're having trouble uploading the project to an ESP32, change your memory partition via Tools -> Partition Scheme -> Huge APP </li>
</ul>
 
