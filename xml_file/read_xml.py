from xml.etree import ElementTree
from deal_xml import DealXml

if __name__ == '__main__':
    xml_instance = DealXml("instance-0000000a.xml")
    #xml_instance.show_content()
    cpuinfo =  xml_instance.get_cpus()
    clock = xml_instance.get_clock()
