
import xml.etree.ElementTree as xml
import tags

class Report:

    def __init__(self, filename):
        self.filename = filename
        self.root = xml.parse(filename).getroot()
        self.version = self.root.get("version")

    def get_filename_with_extension(self, extension):
        if self.filename.endswith(".xml"):
            return self.filename[:-4] + extension
        else:
            self.filename + extension

    def errors(self):
        return list(self.root.findall("error"))

    def section_to_tags(self, tag, element):
        ul = tag.child("ul")
        for e in element:
            ul.child("li", "{0}: {1}".format(e.tag.capitalize(), e.text))

    @property
    def number_of_nodes(self):
        return int(self.root.find("statistics").find("nodes").text)

    def export(self):
        html = tags.Tag("html")
        head = html.child("head")
        head.child("title", "Aislinn report")
        body = html.child("body")
        body.child("h1", "Aislinn report")

        body.child("h2", "Basic information")
        self.section_to_tags(body, self.root.find("program"))

        body.child("h2", "Errors")

        errors = self.errors()
        if errors:
            ul = body.child("ul")
            for error in self.errors():
                li = ul.child("li", error.find("description").text)
                li.set("style", "color: red;")
            body.child("p", "{0} error(s) found".format(len(errors)))
        else:
            body.child("p", "No errors found")

        filename = self.get_filename_with_extension(".html")
        f = open(filename, "w")
        html.write(f)
