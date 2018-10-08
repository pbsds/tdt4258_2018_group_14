#!/usr/bin/env python2
import time, math, os, sys
# this is some re-used old code from an old project,
# do not submit this to the grader!
#
#     pip2 install mido, pyaudio, numpy

forceSoftware = False
if os.name == "nt" and not forceSoftware:
	from winsound import Beep
else:
	import numpy as np, pyaudio

	def Beep(freq, time, cache=[]): # blocks while beeping
		if not cache:
			pa = pyaudio.PyAudio()
			cache.append(pa.open(format=pyaudio.paInt16, channels=1, rate=44100, output=True))
		cache[0].write(np.int16(np.cos(np.arange(int(44100*time/1000.), dtype=np.float)*(math.pi/44100 * freq))*0x3FFF).tostring())

def note2freq(note):
	#return 440.*math.pow(2, float(note-69)/12.)
	return 440.*math.pow(2, float(note-57)/12.)

def note2define(note):
	asd = ("nA", "nBb", "nB", "nC", "nDb", "nD", "nEb", "nE", "nF", "nGb", "nG", "nAb")
	return asd[note%12] + str(note//12)

def playString(data):
	epoch = time.time()
	for i in data.split("\n"):
		if " " in i:
			mtime, note = i.split(" ")

			if not note.isdigit():
				while time.time() < epoch + int(mtime)/1000.:
					pass
				#sleep instead?, nah, i like accuracy
			else:
				f = int(note2freq(int(note)))
				t = int(epoch*1000 + int(mtime) - time.time()*1000) - 1

				print "freq: %s  len(ms): %s  @time(ms): %s" % (str(f).rjust(6), str(t).rjust(4), mtime.rjust(8))
				
				# make it winsound.Beep compliant
				if f < 37: f = 37
				if f >= 32767: f = 32766
				if f % 2: f += 1

				if t > 0:
					Beep(f, t)

def convertMidi2String(path, smart = True):
	import mido
	port =  mido.MidiFile(path)

	time = 0.
	note = None

	out = []#[i] = (mtime, note)
	for msg in port:
		time += msg.time*1000
		if hasattr(msg, "channel") and msg.channel == 9: continue # filter percussion
		out.append((int(time), note))
		if msg.type == "note_on" and msg.velocity <> 0:
			if note:
				if note < msg.note:
					note = msg.note
				else:
					pass
					out.pop()
			else:
				note = msg.note
		elif msg.type in ("note_on", "note_off") and smart:
			if note:
				if msg.note == note:
					note = None
				else:
					pass
					out.pop()
			else:
				#note = None
				if out[-1][1] == None:
					out.pop()
		else:
			if smart:
				out.pop()
			else:
				note = None
				if out[-1][1] == None:
					out.pop()

	#filter crap:
	prev = None
	for i, (mtime, note) in enumerate(out):
		if mtime == prev: # first wins
			out[i] = None
		prev = mtime

	return "\n".join("%i %s" % i for i in out if i)

def convertStringToC(data, name="MY_SONG"):
	out = [
		"#include <stdint.h>",
		"#include \"notes.h\"",
		"",
		"static const uint32_t "+name+"[][2] = { ",
	]
	w  = max(map(lambda x: len(x.split()[0]), data.strip().splitlines()))
	prev = None
	for time, note in map(str.split, data.strip().splitlines()):
		if note == prev:
			out.pop()
		else:
			prev = note
		out.append("\t{%s, %s}," % (time.rjust(w), note2define(int(note)) if note != "None" else "0"))

	out += ["\t{%s, 0},"                              % (int(time)+1)]
	out += ["\t{%s, 0} // double zero terminated :^)" % (int(time)+2)]
	out += ["};"]
	return "\n".join(out)


if __name__ == "__main__":
	if len(sys.argv) <= 1 or sys.argv[1] not in ("--play", "--convert"):
		print "usage:\n\t", __file__, "(--play / --convert) (file)"
	else:
		data = convertMidi2String(sys.argv[2])
		if sys.argv[1] == "--play":
			playString(data)
		elif sys.argv[1] == "--convert":
			print
			print convertStringToC(data,
				name=os.path.basename(sys.argv[2]).upper().replace(".MID", "").replace(" ", "_"))
		else:
			print "wat"
