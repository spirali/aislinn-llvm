
import xml.etree.ElementTree as xml
import tags

class Report:

    def __init__(self, filename):
        self.filename = filename
        self.root = xml.parse(filename).getroot()
        self.version = self.root.get("version")
        self.processes = int(self.root.find("program").find("processes").text)

    @property
    def number_of_nodes(self):
        return int(self.root.find("statistics").find("nodes").text)

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


    def _make_row(self, table, data, classes=None):
        tr = table.child("tr")
        for i, d in enumerate(data):
            td = tr.child("td", d)
            if classes and classes[i]:
                td.set("class", classes[i])


    def export_path(self, element, tag):
        table = tag.child("table")
        self._make_row(table.child("thead"), range(self.processes))
        tbody = table.child("tbody")
        ranks = [ [] for x in xrange(self.processes) ]
        for e in element.findall("action"):
            rank = int(e.get("rank"))
            ranks[rank].append(e)

        step = 0
        while True:
            data = [""] * self.processes
            classes = [None] * self.processes
            for r in xrange(self.processes):
                if len(ranks[r]) <= step:
                    continue
                e = ranks[r][step]
                name = e.get("name")
                data[r] = name
                if name.startswith("W"):
                    classes[r] = "Wait"
                if name.endswith("end"):
                    classes[r] = "Send"
                if name.endswith("ecv"):
                    classes[r] = "Recv"
            if not any(data):
                break
            self._make_row(tbody, data, classes)
            step += 1

    def export(self):
        html = tags.Tag("html")
        head = html.child("head")
        head.child("title", "Aislinn report")
        head.child("style", REPORT_CSS)
        body = html.child("body")
        body.child("h1", "Aislinn report")

        body.child("h2", "Basic information")
        self.section_to_tags(body, self.root.find("program"))

        body.child("h2", "Errors")

        errors = self.errors()
        if errors:
            body.child("p", "{0} error(s) found".format(len(errors)))
            for error in self.errors():
                body.child("h3", "Error: " + error.find("description").text)
                path = error.find("path")
                if path is not None:
                    self.export_path(path, body)
        else:
            body.child("p", "No errors found")

        filename = self.get_filename_with_extension(".html")
        f = open(filename, "w")
        html.write(f)

REPORT_CSS = """
table, tr, td { border: black solid 1px; }
td {
    padding: 0.5em;
    text-align: center;
}

.Wait {
    background-color: orange;
}

.Send {
    background-color: lightgreen;
}

.Recv {
    background-color: lightblue;
}



"""
