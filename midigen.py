import mido
import time

file = mido.MidiFile("badapple.mid")

# print(track)

import winsound

# winsound.Beep(freq,timems)

# max_note = 0
# min_note = 10000


freqs = [12543.85,11839.82,11175.30,10548.08,9956.06,9397.27,8869.84,8372.02,7902.13,7458.62,7040.00,6644.88,6271.93,5919.91,5587.65,5274.04,4978.03,4698.64,4434.92,4186.01,3951.07,3729.31,3520.00,3322.44,3135.96,2959.96,2793.83,2637.02,2489.02,2349.32,2217.46,2093.00,1975.53,1864.66,1760.00,1661.22,1567.98,1479.98,1396.91,1318.51,1244.51,1174.66,1108.73,1046.50,987.77,932.33,880.00,830.61,783.99,739.99,698.46,659.26,622.25,587.33,554.37,523.25,493.88,466.16,440.00,415.30,392.00,369.99,349.23,329.63,311.13,293.66,277.18,261.63,246.94,233.08,220.00,207.65,196.00,185.00,174.61,164.81,155.56,146.83,138.59,130.81,123.47,116.54,110.00,103.83,98.00,92.50,87.31,82.41,77.78,73.42,69.30,65.41,61.74,58.27,55.00,51.91,49.00,46.25,43.65,41.20,38.89,36.71,34.65,32.70,30.87,29.14,27.50,25.96,24.50,23.12,21.83,20.60,19.45,18.35,17.32,16.35,15.43,14.57,13.75,12.98,12.25,11.56,10.91,10.30,9.72,9.18,8.66,
8.18,]
freqs.reverse()

# convert to 30 FPS with 5 min max
def get_timeline(track):

    timelines = [None] * 1000000

    current_time = 0

    time_scale = 0.030994659672178375727323318059

    for msg in track:
        current_time += msg.time * time_scale
        if msg.type == 'note_on':
            note = msg.note
            timelines[int(current_time * 30 / 1000)] = int(freqs[note])
        if msg.type == 'note_off':
            timelines[int(current_time*30/1000)] = 0
    timelines = timelines[:int(current_time*30/1000)]
    cur = 0
    cur_frame = 0
    # print(current_time)
    for i in range(len(timelines)):
        if timelines[i] == None:
            timelines[i] = cur
            cur_frame += 1
        else:
            cur = timelines[i]
            cur_frame = 0
    return timelines

timelines = [None] * len(file.tracks)

for i in range(len(timelines)):
    timelines[i] = get_timeline(file.tracks[i])

# timelines = get_timeline(file.tracks[3])
max_timeline_len = 0
for t in timelines:
    max_timeline_len = max(max_timeline_len, len(t))

timeline = [0] * max_timeline_len

for t in timelines:
    for i in range(len(t)):
        if timeline[i] == 0:
            timeline[i] = t[i] 

print(str(max_timeline_len/30) + "sec got", str(3 * 60 + 39) + " sec needed")
# print(max_note, min_note)

def play_timeline(timeline):
    duration = 0
    freq = 0
    for f in timeline:
        if f == freq:
            duration += 1
        else:
            if freq != 0:
                print(freq)
                winsound.Beep(int(freq),int(duration / 30 * 1000))
            else:
                time.sleep(duration / 30)
            freq = f
            duration = 0

# play_timeline(timeline)

output = """
unsigned short badapple_freqs[] = {
"""
for f in timeline:
    output += "%d," % int(f)

output += "\n};\n"

with open("main/badapple_midi.inc",'w') as f:
    f.write(output)