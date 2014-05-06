
class Tag:

    def __init__(self, name, text=None, **kw):
        self.name = name
        self.attributes = []
        self.childs = []
        self.text = text
        for k in kw:
            self.attributes.append((k, kw[k]))

    def set(self, name, value):
        self.attributes.append((name, value))

    def write(self, f):
        f.write("<{0}".format(self.name))
        for name, value in self.attributes:
            f.write(" {0}='{1}'".format(name, value))
        if self.childs or self.text:
            f.write(">")
            if self.childs:
                f.write("\n")
            for tag in self.childs:
                tag.write(f)
            if self.text:
                f.write(self.text)
            f.write("</{0}>\n".format(self.name))
        else:
            f.write(" />\n")

    def add_child(self, child):
        self.childs.append(child)

    def child(self, *args, **kw):
        tag = Tag(*args, **kw)
        self.add_child(tag)
        return tag
