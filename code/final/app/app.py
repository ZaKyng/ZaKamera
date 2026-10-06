import ZaKnode
import pygame
import random

import tkinter as tk
from tkinter import filedialog
import os

import PIL.Image
import json


window = ZaKnode.Game((800, 1000), __file__, "ZaKamera gallery", fps = 30, screen_ratio = 0.8)
window.fonts.addFont("Main", "assets/font1.ttf", 2)

scene = ZaKnode.Scene("Scene", window, (9, 9, 9))

bars_color = (67, 67, 67)

top_bar = ZaKnode.ColorBlock(scene, (800, 70), bars_color)
ZaKnode.Label(top_bar, "Export file", "Main", "l", (255, 255, 255), offset_str = "center")

view = ZaKnode.ColorBlock(scene, (800, 800), (0, 0, 0), offset = (0, top_bar.size[1]))

bottom_bar = ZaKnode.ColorBlock(scene, (800, window.size[1] - top_bar.size[1] - view.size[1]), bars_color, offset = (0, top_bar.size[1] + view.size[1]))
file_path_label = ZaKnode.Label(bottom_bar, "path..", "Main", "xs", offset = (30, 10))
file_name_label = ZaKnode.Label(bottom_bar, "name..", "Main", "s", offset = (30, 30))


def HSVtoRGB(h, s, v):
    h = h / 360
    s = s / 100
    v = v / 100

    i = int(h * 6)
    f = h * 6 - i
    p = int(v * 255 * (1 - s))
    q = int(v * 255 * (1 - f * s))
    t = int(v * 255 * (1 - (1 - f) * s))
    v = int(v * 255)

    i = i % 6

    if i == 0:
        return (v, t, p)
    if i == 1:
        return (q, v, p)
    if i == 2:
        return (p, v, t)
    if i == 3:
        return (p, q, v)
    if i == 4:
        return (t, p, v)
    if i == 5:
        return (v, p, q)

colors = {"white" : (0, 0), "red" : (0, 100), "green" : (120, 100), "blue" : (240, 100), "yellow" : (60, 100), "cyan" : (180, 100), "magenta" : (300, 100), "random" : (0, 100)}
endings = ["w", "r", "g", "b", "y", "c", "m", "random"]

picked_color = 0
color_picker = ZaKnode.BaseNode(bottom_bar, offset = (30, 70))
ZaKnode.CollisionArea(color_picker, 1, True).addCollisionBlock((30, 30))
ZaKnode.CollisionArea(color_picker, 2, True).addCollisionBlock((30, 30), offset = (220, 0))
color_label = ZaKnode.Label(color_picker, list(colors.keys())[picked_color], "Main", "s", offset = (40, 0))

ZaKnode.ClickObject(color_picker, 1, lambda: changeColor(-1))
ZaKnode.ClickObject(color_picker, 2, lambda: changeColor(1))

inverted = False
invert_label = ZaKnode.Label(bottom_bar, "Snake layers", "Main", "xs", offset = (330, 78))
ZaKnode.CollisionArea(invert_label, 8, True).addCollisionBlock((30, 30), offset = (200, -3))

def invertImage():
    global inverted
    inverted = not inverted
    changePhoto()

ZaKnode.ClickObject(invert_label, 8, invertImage)


mirrored = False
mirror_label = ZaKnode.Label(bottom_bar, "Mirror", "Main", "xs", offset = (330, 40))
ZaKnode.CollisionArea(mirror_label, 5, True).addCollisionBlock((30, 30), offset = (100, -3))

def mirrorImage():
    global mirrored
    mirrored = not mirrored
    changePhoto()

ZaKnode.ClickObject(mirror_label, 5, mirrorImage)



file_path = ""

pick_file_button = ZaKnode.TextBlock(bottom_bar, "Browse..", "Main", "xs", bg_color = (15, 15, 78), padding = 9, offset_str = "top-right", offset = (-15, 20))
ZaKnode.CollisionArea(pick_file_button, 1).addCollisionBlock(pick_file_button.size)


def pickFile():
    global file_path

    root = tk.Tk()
    root.withdraw()

    file_path = filedialog.askopenfilename(
        title="Select a File",
        filetypes=[("ZaKamera photo", "*.zkp *.txt"), ("All Files", "*.*")]
    )

    if not file_path:
        return

    path_text = file_path[-35:]
    if len(file_path) > 35:
        path_text = "..." + path_text

    file_text = os.path.basename(file_path)[:-4] + "-" + endings[picked_color]


    file_path_label.change(text = path_text)
    file_name_label.change(text = file_text)

    changePhoto()

ZaKnode.ClickObject(pick_file_button, 1, pickFile)


button_color = (15, 15, 78)

export_file_button = ZaKnode.TextBlock(bottom_bar, "Export", "Main", "xs", bg_color = button_color, padding = 9, offset_str = "bottom-right", offset = (-15, -20))
ZaKnode.CollisionArea(export_file_button, 1).addCollisionBlock(export_file_button.size)
export_impossible_timer = ZaKnode.Timer(export_file_button, 0.4, lambda: export_file_button.change(bg_color = button_color))

def exportZKP():
    global file_path

    if file_path == "":
        export_file_button.change(bg_color = (255, 0, 0))
        export_impossible_timer.start()
        return

    mode = list(colors.keys())[picked_color]

    with open(file_path, "r") as f:
        data_set = json.load(f)

    min_value = min(min(line) for line in data_set)
    max_value = max(max(line) for line in data_set)
    value_range = max(max_value - min_value, 1)

    width, height = len(data_set[0]), len(data_set)

    pixels = []

    for y in range(height):
        for x in range(width):
            if mirrored:
                if y % 2 == 1 and inverted:
                    value = data_set[y][x]
                else:
                    value = data_set[y][-x - 1]
            else:
                if y % 2 == 1 and inverted:
                    value = data_set[y][(-x - 1)]
                else:
                    value = data_set[y][x]
            value = min(int((value - min_value) * 255 / value_range), 255)
            rgb = HSVtoRGB(colors[mode][0], colors[mode][1], value / 255 * 100)
            pixels.append(rgb)

    img = PIL.Image.new("RGB", (width, height))

    img.putdata(pixels)

    img.save(file_path[:-4] + "-" + endings[picked_color] +".png")


    export_file_button.change(bg_color = (0, 255, 0))
    export_impossible_timer.start()

ZaKnode.ClickObject(export_file_button, 1, exportZKP)


def changeColor(changer = 1):
    global picked_color
    picked_color = (picked_color + changer) % len(colors)
    if list(colors.keys())[picked_color] == "random":
        colors["random"] = (random.randrange(0, 360), 100)
    color_label.change(text = list(colors.keys())[picked_color], color = HSVtoRGB(colors[list(colors.keys())[picked_color]][0], colors[list(colors.keys())[picked_color]][1], 100))

    file_text = os.path.basename(file_path)[:-4] + "-" + endings[picked_color]
    
    file_name_label.change(text = file_text)

    changePhoto()


def changePhoto():
    global file_path, view, picked_color

    if file_path == "":
        return
    
    mode = list(colors.keys())[picked_color]
    
    for block in view.children[:]:
        block.kill()

    with open(file_path, "r") as f:
        data_set = json.load(f)

    min_value = min(min(line) for line in data_set)
    max_value = max(max(line) for line in data_set)
    #pixel_size = min(900 // len(data_set[0]), 900 // len(data_set))
    pixel_size = min(view.size[0] // len(data_set[0]), view.size[1] // len(data_set))
    value_range = max(max_value - min_value, 1)

    def_offset = ((view.size[0] - pixel_size * len(data_set[0])) // 2, (view.size[1] - pixel_size * len(data_set)) // 2)

    for y in range(len(data_set)):
        line = data_set[y]
        for x in range(len(line)):
            if mirrored:
                    if y % 2 == 1 and inverted:
                        value = line[x]
                    else:
                        value = line[-x - 1]
            else:
                if y % 2 == 1 and inverted:
                    value = line[(-x - 1)]
                else:
                    value = line[x]
            value = min(int((value - min_value) * 255 / value_range), 255)
            """
            rgb = []
            for i in mode:
                if i == 0:
                    rgb.append(otherColor(value))
                elif i == 1:
                    rgb.append(mainColor(value))
                else:
                    rgb.append(value)
            """

            rgb = HSVtoRGB(colors[mode][0], colors[mode][1], value / 255 * 100)

            ZaKnode.ColorBlock(view, (pixel_size + 1, pixel_size + 1), rgb, offset = (x * pixel_size + def_offset[0], y * pixel_size + def_offset[1]))



def Main():
    pass

def MainInput(event):
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_ESCAPE:
            window.end()






window.run(Main, MainInput)
