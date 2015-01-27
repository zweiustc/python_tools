VERSION=0.5
default:
	@rm -fr output
	@mkdir output
	@tar zcf output/ssd_op-v${VERSION}.tgz setup Makefile master ssd_op

install:
	python setup/setup.py install

clean:
	@rm -fr output
