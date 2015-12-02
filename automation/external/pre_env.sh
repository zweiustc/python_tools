sh add_pip_repo.sh
easy_install pip
pip install --upgrade pip
yum install -y python-virtualenv openssl-devel python-pip git gcc libffi-devel libxslt-devel mysql-devel postgresql-devel python-setuptools libxml2 python-devel python-psycopg2
pip install -r ../requirements.txt
if [ ! -d /var/log/automation ]; then
  mkdir -p /var/log/automation
fi
