from kivy.app import App

from kivy.uix.floatlayout import FloatLayout
from kivy.uix.scatter import Scatter
from kivy.uix.label import Label
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.image import Image

from kivy.graphics.context_instructions import Rotate

from math import atan2, degrees

class PerricoWidget(BoxLayout):
	def inclinar(self, angulo):
		esparcido=self.ids['esparcido']
		esparcido.rotation += 20*2*angulo/self.width 
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
		return PerricoWidget()

if __name__ == "__main__":
	PerricoApp().run()



