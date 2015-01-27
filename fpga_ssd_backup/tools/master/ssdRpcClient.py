import optparse
import os
import sys
import logging
from xmlrpclib import ServerProxy
from define import *

failList = {}
errorString = {
	NOERR		: 'success',
	ERRMAGIC	: 'ssd driver magic error',
	ERROPER		: 'the type of operation error',
	ERRCHK		: 'change kernel error',
	ERRRMMOD	: 'remove module error',
	ERRINSMOD	: 'insert module error',
	ERRNOFILE	: 'file is not exist',
	ERRNEWCARD	: 'open new card error',
	ERRDOWN		: 'download files error',
	ERRPCI		: 'change PCI error',
	ERRV5V		: 'version of V5 error',
	ERRV5		: 'change V5 error',
	ERRS6		: 'change S6 error',
}

class ssdClient:
	def __init__(self, server, port):
		self.serverHost = server
		self.serverPort = port
		self.serverUrl = 'http://' + self.serverHost + ':' + self.serverPort

	def startProxy(self):
		self.serverProxy = ServerProxy(self.serverUrl)

	def	clientCall(self, request):
		return self.serverProxy.opCall(request)

def ssdGetLogger(logName = 'ssdClient', fileName = LOGPATH):
	if hasattr(ssdGetLogger, "logger"):
		return logger

	logger = logging.getLogger(logName)
	logger.setLevel(logging.DEBUG)

	fh = logging.FileHandler(fileName)
	fh.setLevel(logging.DEBUG)

	sh = logging.StreamHandler()
	sh.setLevel(logging.ERROR)

	formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
	fh.setFormatter(formatter)
	sh.setFormatter(formatter)

	logger.addHandler(fh)
	logger.addHandler(sh)

	return logger

logger = ssdGetLogger()


"""
	function for RPC
"""
def rpcCall(request):
	client = ssdClient(request['server'], request['port'])
	client.startProxy()

	try:
		logger.debug('rpcCall request: %r', request)
		ret = client.clientCall(request)
		logger.debug('rpcCall result: %r', ret['result'])
		print errorString[ret['error']]
		print ret['result']
		
		if NOERR != ret['error']:
			failList[request['server']] = 'error: ', ret['error']
		elif request['server'] in failList.keys():
			del failList[request['server']]
	
	except Exception as ex:
		failList[request['server']] = ex

def rpcMultiCall(requestList):
	logger.debug('rpcMultiCall')
	for each in range(0, len(requestList)):
		rpcCall(requestList[each]);

def reqCreator(server, port, operation, handle):
	return {
		'magic'		: SSDMAGIC,
		'server'	: server,
		'port'		: port,
		'operation'	: operation,
		'handle'	: handle,
	}

def main():
	parser = optparse.OptionParser()
	parser.add_option('-f', '--file', dest = 'file', default = None, help = 'hosts file path')
	parser.add_option('-s', '--server', dest = 'server', default = None, help = 'server host')
	parser.add_option('-p', '--port', dest = 'port', default = str(serverPort), help = 'server port')
	parser.add_option('-o', '--operation', dest = 'operation', default = None, 
			help='operation type: \n' +
				'-o scan, scan host for testing;\n' +
				'-o getkernel, get kernel version of host;\n' +
				'-o chkernel, change kernel;\n' +
				'-o loadssd, load ssd driver;\n' +
				'-o cfgflash, cfg flash;\n' +
				'-o pretools, download tools\n' + 
				'-o chkflash, check flash(V5 + S6)\n' +
				'-o insmod, insmod ssd driver\n' + 
				'-o softlink, link all the tools\n')

	(o, a) = parser.parse_args()

	if not o.operation:
		logger.error('please init operation')
		return -1
		
	try:
		handle = {
			CHKERNEL	: lambda : kernelName,
			CHSSDDRV	: lambda : drvName,
			DOWNLOAD	: lambda : toolsDir,
			CFGFLASH	: lambda : toolsDir,
			CHKFLASH	: lambda : toolsDir,
			INSMODDRV	: lambda : toolsDir,
			LINK		: lambda : toolsDir,
		}[o.operation]()
	except Exception:
		handle = 'NULL'

	if o.file:
		if not os.path.exists(o.file):
			logger.error('file not exist!')
			return -1

		confHandle = open(o.file, 'r')
		serverList = confHandle.readlines()
		requestList = []
		
		for each in serverList:
			each = each.replace('\n', '')
			request = reqCreator(each, o.port, o.operation, handle)
			requestList.append(request)

		rpcMultiCall(requestList)

	elif o.server:
		serverHost = o.server
		request = reqCreator(o.server, o.port, o.operation, handle)
		rpcCall(request)

	else:
		logger.error('args error')
		return -1

	if failList:
		logger.debug('failList: %r', failList)
		print 'failList: ', failList


if '__main__' == __name__:
	main()
