
from oslo_log import log as logging

LOG = logging.getLogger(__name__)


if __name__ == '__main__':
    from automation.clients import Manager
    client_manager = Manager()
    print(client_manager.servers_client.display_default_nova_az())
