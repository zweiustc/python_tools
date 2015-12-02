Automation - The Kingstack Integration Test Suite
==============================================



Quickstart
----------

To run Automation, you first need to create a configuration file that
will tell Automation where to find the various OpenStack services and
other testing behavior switches.

The easiest way to create a configuration file is to generate a sample
in the ``etc/`` directory ::

    $> cd $AUTOMATION_ROOT_DIR
    $> oslo-config-generator --config-file \
        tools/config/config-generator.automation.conf \
        --output-file etc/automation.conf

After that, open up the ``etc/automation.conf`` file and edit the
configuration variables to match valid data in your environment.
This includes your Keystone endpoint, a valid user and credentials,
and reference data to be used in testing.
