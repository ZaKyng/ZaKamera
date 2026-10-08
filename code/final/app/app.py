import ZaKnode
import pygame
import random

import tkinter as tk
from tkinter import filedialog
import os

import PIL.Image
import json

import comps


window = ZaKnode.Game((800, 1000), __file__, "ZaKamera - export", fps = 30, screen_ratio = 0.8)
window.fonts.addFont("Main", window.directory("assets/tommy_bold.ttf"), 2)

icons = ZaKnode.SpriteSheet(window.directory("assets/buttons2.png"), (8, 8), True)


scene = ZaKnode.Scene("Scene", window, (9, 9, 9))

out_of_focus = ZaKnode.ColorBlock(scene, window.size, (0, 0, 0, 128), offset = (0, 0), alpha_channel = True)
ZaKnode.Label(out_of_focus, "Click to continue", "Main", "l", (255, 255, 255), offset_str = "center")

out_of_focus.change(active = False)


bars_color = (67, 67, 67)

top_bar = ZaKnode.ColorBlock(scene, (800, 70), bars_color)
ZaKnode.Label(top_bar, "Export file", "Main", "l", (255, 255, 255), offset_str = "center")

view = ZaKnode.ColorBlock(scene, (800, 800), (0, 0, 0), offset = (0, top_bar.size[1]))
view_error_label = ZaKnode.Label(view, "Error: Invalid JSON file format.", "Main", "s", (255, 0, 0), offset_str = "center", zindex = 10000)
view_error_timer = ZaKnode.Timer(view_error_label, 10, lambda: view_error_label.change(active = False))
view_error_label.change(active = False)

def JSONError():
    global view_error_label, view_error_timer, file_path
    file_path = ""
    view_error_label.change(active = True)
    view_error_timer.start()


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
color_label = ZaKnode.Label(color_picker, list(colors.keys())[picked_color], "Main", "s", offset = (40, 0))

color_button_left = comps.Button(color_picker, (30, 30), icons.grid[0][3], lambda: changeColor(-1), bigger = 2, offset = (30, 20), encor = "right")
color_button_right = comps.Button(color_picker, (30, 30), icons.grid[0][2], lambda: changeColor(1), bigger = 2, offset = (180, 20), encor = "left")

inverted = False
invert_label = ZaKnode.Label(bottom_bar, "Snake layers", "Main", "xs", offset = (350, 78))


def invertImage():
    global inverted, invert_button
    inverted = not inverted

    invert_button.change(img = icons.grid[0][1] if inverted else icons.grid[0][0])

    changePhoto()

invert_button = comps.Button(invert_label, (30, 30), icons.grid[0][0], invertImage, bigger = 3, offset = (150, 12))


mirrored = False
mirror_label = ZaKnode.Label(bottom_bar, "Mirror", "Main", "xs", offset = (350, 34))

def mirrorImage():
    global mirrored, mirror_button
    mirrored = not mirrored

    mirror_button.change(img = icons.grid[0][1] if mirrored else icons.grid[0][0])

    changePhoto()

mirror_button = comps.Button(mirror_label, (30, 30), icons.grid[0][0], mirrorImage, bigger = 3, offset = (120, 12))



file_path = ""

pick_file_button = ZaKnode.TextBlock(bottom_bar, "Browse..", "Main", "xs", bg_color = (15, 15, 78), padding = 6, offset_str = "top-right", offset = (-15, 20))
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

    with open(file_path, "r") as f:
        try:
            data_set = json.load(f)
        except json.JSONDecodeError:
            JSONError()
            return
        
    if not isinstance(data_set, list) or not data_set:
        JSONError()
        return

    if not all(isinstance(row, list) and len(row) > 0 for row in data_set):
        JSONError()
        return

    first_row_len = len(data_set[0])
    if not all(len(row) == first_row_len for row in data_set):
        JSONError()
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

export_file_button = ZaKnode.TextBlock(bottom_bar, "Export", "Main", "xs", bg_color = button_color, padding = 6, offset_str = "bottom-right", offset = (-15, -20))
ZaKnode.CollisionArea(export_file_button, 1).addCollisionBlock(export_file_button.size)
export_impossible_timer = ZaKnode.Timer(export_file_button, 1.2, lambda: export_file_button.change(bg_color = button_color))

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

    if file_path == "":
        return

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
        try:
            data_set = json.load(f)
        except json.JSONDecodeError:
            JSONError()
            return

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
    if not pygame.key.get_focused():
        out_of_focus.change(active = True)
    else:
        out_of_focus.change(active = False)

def MainInput(event):
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_ESCAPE:
            window.end()






window.run(Main, MainInput)
