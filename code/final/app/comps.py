import ZaKnode

class Button:
    def __init__(self, parent, size, img, func, bigger = 5, encor = "center", offset = (0, 0)):
        self.parent = parent
        self.size = size
        self.orig_size = size
        self.img = img
        self.func = func
        self.bigger = bigger
        self.encor = encor
        self.offset = offset

        self.base = ZaKnode.BaseNode(self.parent, offset = self.offset)
        self.sprite = ZaKnode.SpriteBlock(self.base, self.size, self.img, offset_str = self.encor)
        self.collisionArea = ZaKnode.CollisionArea(self.base, 1)
        self.collider = self.collisionArea.addCollisionBlock(self.size, offset_str = self.encor)

        ZaKnode.Hover(self.base, 1, lambda: self.change(size = (self.orig_size[0] + self.bigger, self.orig_size[1] + self.bigger)), lambda: self.change(size = self.orig_size))
        ZaKnode.ClickObject(self.base, 1, self.click, False)

    def change(self, img = None, size = None):
        if img:
            self.img = img
        if size:
            self.size = size

        self.base.change(offset = self.offset)
        self.sprite.change(image = self.img, size = self.size, offset_str = self.encor)

    def click(self):
        self.func()