import os
import sys
import logging
import optparse
import time
from SimpleXMLRPCServer import SimpleXMLRPCServer
from daemon2x import Daemon
from define import *

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
	ERRCHECK	: 'check ssd error',
	ERRGETEC	: 'get erase count from huawei error',
	ERRSETEC	: 'set erase count to baidu error',
}

def ssdGetLogger(logName = 'ssdServer', fileName = LOGPATH):
	if hasattr(ssdGetLogger, "logger"):
		return logger
		
	logger = logging.getLogger(logName)
	logger.setLevel(logging.DEBUG)

	fh = logging.FileHandler(fileName)
	fh.setLevel(logging.DEBUG)

	formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
	fh.setFormatter(formatter)

	logger.addHandler(fh)

	return logger

class ssdRpc:
	def __init__(self, port):
		serverHost = os.popen('hostname')
		self.serverHost = serverHost.read().replace('\n', '')
		self.serverPort = port
		self.logger = ssdGetLogger()
	
	def rpcStart(self):
		self.logger.debug('rpcServer start......')
		self.server = SimpleXMLRPCServer((self.serverHost, self.serverPort))
		self.server.register_function(self.opCall)
		self.server.serve_forever()

	def cmdRun(self, cmd, needResult = True):
		self.logger.debug('cmdRun: %r', cmd)
		result = os.popen(cmd)

		if needResult:
			result = result.read().replace('\n', '')
#			self.logger.debug('cmd: %r, result: %r', cmd, result)
			return result

	def hostScan(self):
		return {
			'magic'     : SSDMAGIC,
			'result'    : 0,
			'error'     : NOERR
		}

	def getKernel(self):
		cmd = 'uname -r'
		kernel = self.cmdRun(cmd, True)
		
		return {
			'magic'		: SSDMAGIC,
			'result'	: (self.serverHost, kernel),
			'error'		: NOERR
		}
		
	def changeKernel(self, kernelName):
		fileName = kernelName + '.tgz'
		if os.path.exists(fileName):
			self.logger.debug('%s exists, we remove it', fileName)
			cmd = 'rm -fr ' + fileName
			self.cmdRun(cmd, False)
			cmd = 'rm -fr ' + kernelName
			self.cmdRun(cmd, False)

		path = os.path.join(kernelUrl, fileName)
		cmd = 'wget ' + path + ' 2>/dev/null 1>/dev/null'
		self.cmdRun(cmd, False)

		if not os.path.exists(fileName):
			self.logger.debug('fail to download %s', fileName)
			return {
				'magic'     : SSDMAGIC,
				'result'    : -1,
				'error'		: ERRCHK
			}

		cmd = 'tar zxf ' + fileName + ' && cd ' + kernelName + ' && ./auto_changekernel.pl 1>/dev/null 2>/dev/null'
		self.cmdRun(cmd, False)

		self.logger.debug('We need to modify grub and reboot after change kernel')

		cmd = 'sed -i \'s/default=[^0]/default=0/g\' /boot/grub/grub.conf'
		self.cmdRun(cmd, False)

		#reboot after 1m
		cmd = 'shutdown -r +1 &'
		self.cmdRun(cmd, False)
		return {
			'magic'     : SSDMAGIC,
			'result'    : 0,
			'error'     : NOERR
		}

	def checkSsdMod(self, modName):
		cmd = 'lsmod | grep ' + modName
		return not not self.cmdRun(cmd, True)

	def checkDevice(self):
		#check /dev/ssd
		TIMES = 5
		timesTry = 0
		while timesTry < TIMES:
			cmd_test = 'ls /dev | grep ssd_Baidu43'
			if self.cmdRun(cmd_test, True):
				break

			timesTry += 1
			time.sleep(3)

		if timesTry == TIMES:
			self.logger.debug('scan device timeout')
			return {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERRINSMOD
			}

		time.sleep(3)
		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}

	def insmodSsdDriver(self, rootDir):
		drvName = 'ssd_drv.ko'
		modName = drvName.replace('.ko', '')
		if self.checkSsdMod(modName):
			self.logger.debug('we need to rmmod driver')
			cmd = 'rmmod ' + modName
			if self.cmdRun(cmd, True):
				self.logger.debug('fail to rmmod driver')
				return {
					'magic'		: SSDMAGIC,
					'result'	: -1,
					'error'		: ERRRMMOD
				}
		
		drvPath = os.path.join(rootDir, drvName)
		if not os.path.exists(drvPath):
			return self.loadSsdDriver(drvName, rootDir)

		cmd = 'insmod ' + drvPath
		self.cmdRun(cmd, False)
		"""
		cmd = 'modprobe ' + modName
		res = self.cmdRun(cmd, True)

		if 'FATAL' in res:
			res = self.loadSsdDriver(drvName, rootDir)
			if NOERR != res['error']:
				return res
		"""

		if not self.checkSsdMod(modName):
			self.logger.debug('fail to insmod driver')
			return {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERRINSMOD
			}

		res = self.checkDevice()
		if NOERR != res['error']:
			return res

		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}

	def loadSsdDriver(self, drvName = 'ssd_drv.ko', rootDir = None):
		ssdName = drvName.replace('.ko', '')
		""" 
		if ssd driver is exists, we rmmod it
		"""
		if self.checkSsdMod(ssdName):
			self.logger.debug('we need to rmmod driver')
			cmd = 'rmmod ' + ssdName
			if self.cmdRun(cmd, True):
				self.logger.debug('fail to rmmod driver')
				return {
					'magic'		: SSDMAGIC,
					'result'	: -1,
					'error'		: ERRRMMOD
				}
				
		"""
		now we can insmod ssd driver(maybe named by driverName)
		"""
		if os.path.exists(drvName):
			self.logger.debug('%s exists, we remove it', drvName)
			cmd = 'rm -fr ' + drvName
			self.cmdRun(cmd, False)

		url = os.path.join(toolsUrl, drvName)
		cmd = 'wget ' + url + ' 2>/dev/null 1>/dev/null'
		self.cmdRun(cmd, False)
		if not os.path.exists(drvName):
			self.logger.debug('driver is not exist')
			return {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERRNOFILE
			}
		"""
		cmd = 'insmod ' + drvName
		self.cmdRun(cmd, False)

		if rootDir:
			cmd = 'cp -f --reply=yes ' + drvName + ' ' + rootDir
			self.cmdRun(cmd)
		"""
		cmd = 'uname -r'
		path = self.cmdRun(cmd, True)
		modPath = '/lib/modules/' + path + '/kernel/drivers/block/'

		cmd = 'cp -f --reply=yes ' + drvName + ' ' + modPath
		self.cmdRun(cmd, False)

		cmd = 'depmod -a'
		self.cmdRun(cmd, False)

		cmd = 'modprobe ' + ssdName
		self.cmdRun(cmd, False)
		#check module
		if not self.checkSsdMod(ssdName):
			self.logger.debug('fail to insmod driver')
			return {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERRINSMOD
			}

		res = self.checkDevice()
		if NOERR != res['error']:
			return res

		#success
		self.logger.debug('success to insmod driver')
		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}

	def downLoads(self, rootDir):
		fileList = ['flash_tools.tar.gz', 'bridge', 'controller', 'op_tools.tar', 'ssd_drv.ko']
		self.logger.debug('download files: %r', fileList)

		cmd = 'mkdir -p ' + rootDir
		self.cmdRun(cmd, False)

		for file in fileList:
			if os.path.exists(file):
				self.logger.debug('%s exists, we remove it', file)
				cmd = 'rm -fr ' + file
				self.cmdRun(cmd, False)

			url = os.path.join(toolsUrl, file)
			cmd = 'wget ' + url + ' 2>/dev/null 1>/dev/null'
			self.cmdRun(cmd, False)
			if not os.path.exists(file):
				self.logger.debug('%s is not exist', file)
				return {
					'magic'		: SSDMAGIC,
					'result'	: -1,
					'error'		: ERRDOWN
				}

			cmd = 'mv -f ' + file + ' ' + rootDir
			self.cmdRun(cmd, False)
		
		cmd = 'cd ' + rootDir + ' && tar zxf '
		cmd_1 = cmd + 'flash_tools.tar.gz'
		self.cmdRun(cmd_1, False)

		cmd_2 = cmd + 'op_tools.tar'
		self.cmdRun(cmd_2, False)

		return {
			'magic'		: SSDMAGIC,
			'result'	: fileList,
			'error'		: NOERR
		}

	def cfgFlash(self, rootDir):
		fileList = ['flash_tools', 'bridge', 'controller']
		for file in fileList:
			fileName = os.path.join(rootDir, file)
			if not os.path.exists(fileName):
				res = self.downLoads(rootDir)
				if NOERR != res['error']:
					return res

		#get erase count from huawei
		res = self.getEcFromHuawei(rootDir)
		if NOERR != res['error']:
			return res

		cmd = 'cd ' + rootDir + ' '
		cmd += '&& mv bridge controller flash_tools/huawei-mlc-3G/ssd_utils '
		cmd += '&& cd flash_tools/huawei-mlc-3G/ssd_utils '
		cmd += '&& ./ssd_fwcrc -f . 2>/dev/null 1>/dev/null '
		cmd += '&& ./ssd_firmware -d /dev/ssda -f ssd_fw 2>/dev/null 1>/dev/null '
		self.cmdRun(cmd, False)

		cmd = 'init 6'
		self.cmdRun(cmd, False)
		
		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}

	def reloadS6(self, rootDir, checkFlash = False):
		cmd = 'cd ' + rootDir + ' '
		#step 2: check V5
		cmd_v5_check = cmd + '&& ./wr_n 0x80f8 0 '
		self.cmdRun(cmd_v5_check, False)

		#step 3: reset
		cmd_v5_reset = cmd + '&& ./wr_n 0x80f8 1'
		self.cmdRun(cmd_v5_reset, False)

		cmd_v5_check = cmd + '&& ./rd_n 0x8068'
		res = self.cmdRun(cmd_v5_check, True)
		lastChar = res[-1]

		#unlikely
		if '0' != lastChar and checkFlash:
			self.logger.debug('check V5 error: %r', res)
			cmd_s6 = cmd + '&& ./cfg_flash_n final_new.mcs '
			self.cmdRun(cmd_s6, False)
			cmd_v5_n = cmd + '&& ./cfg_v5_n '
			self.cmdRun(cmd_v5_n, False)
			####reboot?
			self.cmdRun('reboot', False)

			return {
			'magic'		: SSDMAGIC,
			'result'	: res,
			'error'		: ERRV5
			}
			
		cmd_s6 = cmd + '&& ./cfg_s6_n_40 1 '
		res = self.cmdRun(cmd_s6, True)
		
		self.logger.debug('load S6: %r', res)
		if 'sucess' not in res:
			self.logger.debug('load S6 error: %r', res)

			return {
			'magic'		: SSDMAGIC,
			'result'	: res,
			'error'		: ERRS6
			}

		#init newcard
		cmd_init = cmd + '&& ./open_newcard'
		res = self.cmdRun(cmd_init, True)
		self.logger.debug('open new card: %r', res)
		if 'PASS' not in res:
			self.logger.debug('open newcard error: %r', res)
			return {
				'magic'		: SSDMAGIC,
				'result'	: res,
				'error'		: ERRNEWCARD
			}
		
		#success
		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}
		
	def checkFlash(self, rootDir):
		#check pci
		cmd_pci = 'lspci | grep \'RAM memory\''
		res = self.cmdRun(cmd_pci, True)
		if not 'RAM memory' in res:
			self.logger.debug('change pci error: %r', res)
			return {
				'magic'		: SSDMAGIC,
				'result'	: res,
				'error'		: ERRPCI
			}
		
		#load ssd_drv-ne.ko
		ssdChkName = 'ssd_drv'
		if self.checkSsdMod(ssdChkName):
			self.logger.debug('we need to rmmod driver')
			cmd_chk = 'rmmod ' + ssdChkName
			if self.cmdRun(cmd_chk, True):
				self.logger.debug('fail to rmmod driver')
				return {
					'magic'		: SSDMAGIC,
					'result'	: -1,
					'error'		: ERRRMMOD
				}

		cmd = 'cd ' + rootDir + ' '
		"""
		cmd_tar = cmd + '&& tar zxf op_tools.tar '
		self.cmdRun(cmd_tar, False)
		"""
		cmd += '&& cd op_tools '
		cmd_insmod = cmd + '&& insmod ssd_drv-ne.ko'
		self.cmdRun(cmd_insmod, False)

		if not self.checkSsdMod(ssdChkName):
			self.logger.debug('fail to insmod driver')
			return {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERRINSMOD
			}

		res = self.checkDevice()
		if NOERR != res['error']:
			return res

		#load S6
		cmd_s6 = cmd + '&& ./cfg_s6_n_40 1 '
		res = self.cmdRun(cmd_s6, True)

		self.logger.debug('load S6: %r', res)
		if 'sucess' not in res:
			self.logger.debug('load S6 error: %r', res)

			return {
			'magic'		: SSDMAGIC,
			'result'	: res,
			'error'		: ERRS6
			}
		
		cmd_s6_2 = cmd + '&& ./wr_n 0x80e8 1 '
		self.cmdRun(cmd_s6_2, False)
		#step 1: check version of V5
		cmd_v5_version = cmd + '&& ./rd_n 0x8000'
		res = self.cmdRun(cmd_v5_version, True)
		#get the lastest charactor, expect: '9'
		lastChar = res[-1]
		if '9' != lastChar:
			self.logger.debug('check version of V5 error: %r', res)
			#########reboot?
			self.cmdRun('reboot', False)

			return {
			'magic'		: SSDMAGIC,
			'result'	: res,
			'error'		: ERRV5V
			}
		
		cmd_v5 = cmd + '&& ./rd_n 0x8068 '
		res = self.cmdRun(cmd_v5, True)

		self.logger.debug('check V5: %r', res)
		if '0' != res[-1]:
			cmd_v5_load = cmd + '&& ./cfg_v5_n '
			self.cmdRun(cmd_v5_load, False)

			self.cmdRun('reboot', False)

			return {
			'magic'		: SSDMAGIC,
			'result'	: res,
			'error'		: ERRV5
			}

		self.logger.debug('open new card for Baidu SSD')
		cmd_init = cmd + '&& ./open_newcard'
		res = self.cmdRun(cmd_init, True)
		if 'PASS' not in res:
			self.logger.debug('open newcard error: %r', res)
			dir = os.path.join(rootDir, 'op_tools')
			res = self.reloadS6(dir, True)

			if NOERR != res['error']:
				self.logger.debug('check V5 and S6 error, errorno: %r', res['error'])
				if ERRNEWCARD == res['error']:
					res = self.reloadS6(dir, False)
					#unlikely
					if NOERR != res['error']:
						cmd_v5_load = cmd + '&& ./cfg_v5_n '
						self.cmdRun(cmd_v5_load, False)

						self.cmdRun('reboot', False)
						return res
				else:
					return res

		#replace ssd driver
		cmd_rmmod = 'rmmod ssd_drv'
		self.cmdRun(cmd_rmmod, False)
		#get udev version
		res = self.cmdRun('udev -V', True)
		if int(res) < 70:
			udev_name = 'udev-070.tar.gz'
			cmd_udev = 'wget ' + udevUrl + udev_name + ' >/dev/null 2>/dev/null'
			self.cmdRun(cmd_udev, False)
			cmd_udev = 'tar zxf ' + udev_name + ' && cd udev-070 && make >/dev/null 2>/dev/null && make install >/dev/null 2>/dev/null && cd .. && rm udev-070* -fr'
			self.cmdRun(cmd_udev, False)
			self.logger.debug('update udev version to 070')

		#copy rules
		cmd_rules = cmd + '&& cp -f --reply=yes 60-BaiduSSD.rules /etc/udev/rules.d/ '
		self.cmdRun(cmd_rules, False)
		
		res = self.insmodSsdDriver(rootDir)
		if NOERR != res['error']:
			self.logger.debug('load driver error: %r', res)
			return res

		#init newcard again
		self.logger.debug('open new card for new Baidu SSD')
		cmd_init = cmd + '&& ./init_newcard'
		res = self.cmdRun(cmd_init, True)
		if 'PASS' not in res:
			self.logger.debug('open newcard error: %r', res)
			return {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERRNEWCARD
			}
		"""
		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}
		"""
		#set erase table to baidu SSD
		res = self.setEcToBaidu(rootDir)
		return res

	def softLinkTools(self, rootDir):
		dir = os.path.join(rootDir, 'op_tools')
		softLinks = ['ssd_error', 'ssd_ls', 'ssd_stat', 'ssd_read_id', 'ssd_read_table', 'util_erasetable', 'close_force', 'ssd_mark_bad_block', 'ssd_erase_id', 'ssd_mark_erase_count']

		for link in softLinks:
			src = os.path.join(dir, link)
			dst = os.path.join('/bin', link)
			cmd = 'ln -s ' + src + ' ' + dst

			res = self.cmdRun(cmd, True)
			if 'exists' in res:
				self.logger.debug('%s exists: %r', link, res)

		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}
	
	def depMod(self, rootDir):
		ssdName = 'ssd_drv.ko'
		ssdPath = os.path.join(rootDir, ssdName)

		if not os.path.exists(ssdPath):
			self.insmodSsdDriver(rootDir)
		else:
			cmd = 'uname -r'
			path = self.cmdRun(cmd, True)
			modPath = '/lib/modules/' + path + '/kernel/drivers/block/'

			cmd = 'cp -f --reply=yes ' + ssdPath + ' ' + modPath
			self.cmdRun(cmd, False)

			cmd = 'depmod -a'
			self.cmdRun(cmd, False)

		"""add ssd driver to rc.local"""
		cmd = 'sed -i \'/modprobe ssd_drv/d\' /etc/rc.d/rc.local'
		self.cmdRun(cmd, False)
		cmd = 'echo modprobe ssd_drv >> /etc/rc.d/rc.local'
		self.cmdRun(cmd, False)
		#reload ssd driver
		self.cmdRun('rmmod ssd_drv', False)
		self.cmdRun('modprobe ssd_drv', False)

		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}

	def checkSsd(self, rootDir):
		res = self.insmodSsdDriver('ssd_drv')
		if res['error'] != NOERR:
			self.logger.debug('insmod driver error')
			return {
				'magic'		: SSDMAGIC,
				'result'	: res,
				'error'		: ERRCHECK
			}

		toolPath = os.path.join(rootDir, 'op_tools')	
		cmd = 'cd ' + toolPath + ' && ./ssd_check_all > ssd_check.log'

		self.cmdRun(cmd, False)

		cmd_cat = 'cd ' + toolPath + ' && cat ssd_check.log'
		res = self.cmdRun(cmd_cat, True)
		if 'error' not in res:
			return {
				'magic'		: SSDMAGIC,
				'result'	: 0,
				'error'		: NOERR
			}
		else:
			return {
				'magic'		: SSDMAGIC,
				'result'	: res,
				'error'		: ERRCHECK
			}
			
	def getEcFromHuawei(self, rootDir):
		tools = os.path.join(rootDir, 'flash_tools')
		table = 'erase_table_huawei'
		if not os.path.exists(tools):
			self.logger.debug('we need download tools')
			self.downLoads(rootDir)

		huawei_tools_path = rootDir + '/flash_tools/huawei-mlc-3G/ssd_utils'
		table_path = os.path.join(rootDir, table)
		cmd = 'cd ' + huawei_tools_path + ' && ./ssd_ecinfo -d /dev/ssda > ' + table_path
		self.cmdRun(cmd, False)

		cmd = 'cat ' + table_path + ' | wc -l'
		res = self.cmdRun(cmd, True)
		if res != '45056':
			self.logger.debug('erase table error')
			return {
				'magic'		: SSDMAGIC,
				'result'	: res,
				'error'		: ERRGETEC
			}

		self.logger.debug("get erase count from huawei: %r", table_path)
		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}

	def setEcToBaidu(self, rootDir):
		table = 'erase_table_huawei'
		table_path = os.path.join(rootDir, table)
		if not os.path.exists(table_path):
			self.logger.debug('%r is not exist', table_path)
			return {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERRGETEC
			}

		cmd_path = os.path.join(rootDir, 'op_tools')
		cmd_path = os.path.join(cmd_path, 'ssd_mark_erase_count')

		cmd = cmd_path + ' -f ' + table_path
		res = self.cmdRun(cmd, True)

		if 'fail' in res:
			self.logger.debug("fail to set erase count to baidu SSD")
			return {
				'magic'		: SSDMAGIC,
				'result'	: res,
				'error'		: ERRSETEC
			}

		self.logger.debug('success to set erase count to baidu SSD')
		return {
			'magic'		: SSDMAGIC,
			'result'	: 0,
			'error'		: NOERR
		}

	def opCall(self, request):
		self.logger.debug('recieve request: %r', request)
		reply = None
		if SSDMAGIC != request['magic']:
			reply = {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERRMAGIC
			}
			self.logger.debug('send reply: %r', reply)
			return reply
		
		try:	
			reply = {
				SCAN		: lambda x	: self.hostScan(),
				GETKERNEL	: lambda x	: self.getKernel(),
				CHKERNEL	: lambda x	: self.changeKernel(x),
				CHSSDDRV	: lambda x	: self.loadSsdDriver(x),
				CFGFLASH	: lambda x	: self.cfgFlash(x),
				DOWNLOAD	: lambda x	: self.downLoads(x),
				CHKFLASH	: lambda x	: self.checkFlash(x),
				INSMODDRV	: lambda x	: self.insmodSsdDriver(x),
				LINK		: lambda x	: self.softLinkTools(x),
				DEPDRIVER	: lambda x	: self.depMod(x),
				CHECKSSD	: lambda x	: self.checkSsd(x),
				GETEC		: lambda x	: self.getEcFromHuawei(x),
				SETEC		: lambda x	: self.setEcToBaidu(x),
			}[request['operation']](request['handle'])
		except Exception:
			reply = {
				'magic'		: SSDMAGIC,
				'result'	: -1,
				'error'		: ERROPER
			}

		self.logger.debug('send reply: %r', reply)
		return reply

class ssdServerDaemon(Daemon):
	def run(self):
		ssdServer = ssdRpc(serverPort)
		ssdServer.rpcStart()

def reqCreator(server, port, operation, handle):
	return {
		'magic'		: SSDMAGIC,
		'server'	: server,
		'port'		: port,
		'operation'	: operation,
		'handle'	: handle,
	}

if '__main__' == __name__:
	parser = optparse.OptionParser()
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
				'-o softlink, link all tools\n' +
				'-o depmod, depend ssd dirver\n' +
				'-o checkssd, check ssd\n' +
				'-o getec, get erase count from huawei SSD\n' +
				'-o setec, set erase count to baidu SSD')
	(o, a) = parser.parse_args()

	try:
		handle = {
			CHKERNEL	: lambda : kernelName,
			CHSSDDRV	: lambda : drvName,
			DOWNLOAD	: lambda : toolsDir,
			CFGFLASH	: lambda : toolsDir,
			CHKFLASH	: lambda : toolsDir,
			INSMODDRV	: lambda : toolsDir,
			LINK		: lambda : toolsDir,
			DEPDRIVER	: lambda : toolsDir,
			CHECKSSD	: lambda : toolsDir,
			GETEC		: lambda : toolsDir,
			SETEC		: lambda : toolsDir,
		}[o.operation]()
	except Exception:
		handle = 'NULL'

	request = reqCreator(None, None, o.operation, handle)
	rpc = ssdRpc(None)
	reply = rpc.opCall(request)

	print 'result: %r' % errorString[reply['error']]
	print 'reply is: %r' % reply
