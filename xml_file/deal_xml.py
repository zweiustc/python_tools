from xml.etree import ElementTree

import os

class DealXml(object):
    def __init__(self, filename, *args, ** kwargs):
        super(DealXml, self).__init__(*args, ** kwargs)
        self.filename = filename
        text = open(self.filename).read()
        self.content =  ElementTree.fromstring(text)

    def show_content(self):
        if self.filename:
            text = open(self.filename).read()
            print text

    def get_cpus(self):
        vcpus = self.content.getiterator("vcpus")
        return vcpus

    def get_clock(self):
        clock = self.content.getiterator("clock")
        return clock
