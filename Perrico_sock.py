from kivy.app import App

from kivy.uix.floatlayout import FloatLayout
from kivy.uix.scatter import Scatter
from kivy.uix.label import Label
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.image import Image

from kivy.graphics.context_instructions import Rotate

from math import atan2, degrees

import socket
from kivy.clock import Clock
import re

class PerricoWidget(BoxLayout):
	#Socket para conectarse al router openwrt
	s = socket.socket()
	#Patron para extraer un angulo de la cadena leida
	#patron = re.compile("[-]{0,1}[0-9]{1,}.[0-9]{2}")
	patron = re.compile("Z:[-]{,1}[0-9]{1,}.[0-9]{2}")
	def leer(self,dt):
		cartel=self.ids['cartel']
		recibido = self.s.recv(40)
		c = self.patron.search(recibido)
		if c:
			a = c.group(0)
			a = a[2:]
			cartel.text = a
			self.inclinar_a(float(a))
	def inclinar(self, angulo):
		esparcido=self.ids['esparcido']
		esparcido.rotation += 20*2*angulo/self.width 
	def inclinar_a(self, angulo):
		esparcido=self.ids['esparcido']
		esparcido.rotation = angulo 
	def on_touch_down(self, touch):
		cartel=self.ids['cartel']
		esparcido=self.ids['esparcido']
		toque_x = touch.x-self.center_x
		if toque_x>0:
			sig = +1
		else:
			sig = -1
		#angulo = sig*degrees(atan2(abs(touch.x-self.center_x), touch.y-self.center_y))
		if esparcido.rotation > 180:
			angulo = esparcido.rotation - 360
		else:
			angulo = esparcido.rotation
		cartel.text = str(round(angulo,2))
		self.inclinar(-toque_x)

class PerricoApp(App):
	def build(self):
		perrico = PerricoWidget()
		perrico.s.connect(("192.168.2.1", 1234))
		Clock.schedule_interval(perrico.leer,0.001) 
		return perrico

if __name__ == "__main__":
	PerricoApp().run()



