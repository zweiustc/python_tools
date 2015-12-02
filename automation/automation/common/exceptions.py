# Copyright 2012 OpenStack Foundation
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

import testtools


class AutomationException(Exception):
    """Base Tempest Exception

    To correctly use this class, inherit from it and define
    a 'message' property. That message will get printf'd
    with the keyword arguments provided to the constructor.
    """
    message = "An unknown exception occurred"

    def __init__(self, *args, **kwargs):
        super(AutomationException, self).__init__()
        try:
            self._error_string = self.message % kwargs
        except Exception:
            # at least get the core message out if something happened
            self._error_string = self.message
        if len(args) > 0:
            # If there is a non-kwarg parameter, assume it's the error
            # message or reason description and tack it on to the end
            # of the exception message
            # Convert all arguments into their string representations...
            args = ["%s" % arg for arg in args]
            self._error_string = (self._error_string +
                                  "\nDetails: %s" % '\n'.join(args))

    def __str__(self):
        return self._error_string


class RestClientException(AutomationException,
                          testtools.TestCase.failureException):
    pass


class OtherRestClientException(RestClientException):
    pass


class ServerRestClientException(RestClientException):
    pass


class ClientRestClientException(RestClientException):
    pass


class InvalidHttpSuccessCode(OtherRestClientException):
    message = "The success code is different than the expected one"


class NotFound(ClientRestClientException):
    message = "Object not found"


class Unauthorized(ClientRestClientException):
    message = 'Unauthorized'


class Forbidden(ClientRestClientException):
    message = "Forbidden"


class TimeoutException(OtherRestClientException):
    message = "Request timed out"


class BadRequest(ClientRestClientException):
    message = "Bad request"


class UnprocessableEntity(ClientRestClientException):
    message = "Unprocessable entity"


class RateLimitExceeded(ClientRestClientException):
    message = "Rate limit exceeded"


class OverLimit(ClientRestClientException):
    message = "Quota exceeded"


class ServerFault(ServerRestClientException):
    message = "Got server fault"


class NotImplemented(ServerRestClientException):
    message = "Got NotImplemented error"


class Conflict(ClientRestClientException):
    message = "An object with that identifier already exists"


class ResponseWithNonEmptyBody(OtherRestClientException):
    message = ("RFC Violation! Response with %(status)d HTTP Status Code "
               "MUST NOT have a body")


class ResponseWithEntity(OtherRestClientException):
    message = ("RFC Violation! Response with 205 HTTP Status Code "
               "MUST NOT have an entity")


class InvalidHTTPResponseBody(OtherRestClientException):
    message = "HTTP response body is invalid json or xml"


class InvalidHTTPResponseHeader(OtherRestClientException):
    message = "HTTP response header is invalid"


class InvalidContentType(ClientRestClientException):
    message = "Invalid content type provided"


class UnexpectedContentType(OtherRestClientException):
    message = "Unexpected content type provided"


class UnexpectedResponseCode(OtherRestClientException):
    message = "Unexpected response code received"


class InvalidStructure(AutomationException):
    message = "Invalid structure of table with details"


class BadAltAuth(AutomationException):
    """Used when trying and failing to change to alt creds.

    If alt creds end up the same as primary creds, use this
    exception. This is often going to be the case when you assume
    project_id is in the url, but it's not.

    """
    message = "The alt auth looks the same as primary auth for %(part)s"


class CommandFailed(Exception):
    def __init__(self, returncode, cmd, output, stderr):
        super(CommandFailed, self).__init__()
        self.returncode = returncode
        self.cmd = cmd
        self.stdout = output
        self.stderr = stderr

    def __str__(self):
        return ("Command '%s' returned non-zero exit status %d.\n"
                "stdout:\n%s\n"
                "stderr:\n%s" % (self.cmd,
                                 self.returncode,
                                 self.stdout,
                                 self.stderr))


class IdentityError(AutomationException):
    message = "Got identity error"


class EndpointNotFound(AutomationException):
    message = "Endpoint not found"


class InvalidCredentials(AutomationException):
    message = "Invalid Credentials"


class SSHTimeout(AutomationException):
    message = ("Connection to the %(host)s via SSH timed out.\n"
               "User: %(user)s, Password: %(password)s")


class SSHExecCommandFailed(AutomationException):
    """Raised when remotely executed command returns nonzero status."""
    message = ("Command '%(command)s', exit status: %(exit_status)d, "
               "stderr:\n%(stderr)s\n"
               "stdout:\n%(stdout)s")


class RestClientException(AutomationException,
                          testtools.TestCase.failureException):
    pass


class InvalidConfiguration(AutomationException):
    message = "Invalid Configuration"


class InvalidServiceTag(AutomationException):
    message = "Invalid service tag"


class InvalidIdentityVersion(AutomationException):
    message = "Invalid version %(identity_version) of the identity service"


class BuildErrorException(AutomationException):
    message = "Server %(server_id)s failed to build and is in ERROR status"


class ImageKilledException(AutomationException):
    message = "Image %(image_id)s 'killed' while waiting for '%(status)s'"


class AddImageException(AutomationException):
    message = "Image %(image_id)s failed to become ACTIVE in the allotted time"


class EC2RegisterImageException(AutomationException):
    message = ("Image %(image_id)s failed to become 'available' "
               "in the allotted time")


class VolumeBuildErrorException(AutomationException):
    message = "Volume %(volume_id)s failed to build and is in ERROR status"


class VolumeRollbackErrorException(AutomationException):
    message = "Volume %(volume_id)s failed to rollback"


class VolumeRestoreErrorException(AutomationException):
    message = "Volume %(volume_id)s failed to restore and is in ERROR status"


class SnapshotBuildErrorException(AutomationException):
    message = "Snapshot %(snapshot_id)s failed to build and is in ERROR status"


class SnapshotDeleteException(AutomationException):
    message = "Snapshot failed to be deleted"


class VolumeBackupException(AutomationException):
    message = "Volume backup %(backup_id)s failed and is in ERROR status"


class StackBuildErrorException(AutomationException):
    message = ("Stack %(stack_identifier)s is in %(stack_status)s status "
               "due to '%(stack_status_reason)s'")


class StackResourceBuildErrorException(AutomationException):
    message = ("Resource %(resource_name)s in stack %(stack_identifier)s is "
               "in %(resource_status)s status due to "
               "'%(resource_status_reason)s'")


class AuthenticationFailure(AutomationException):
    message = ("Authentication with user %(user)s and password "
               "%(password)s failed auth using tenant %(tenant)s.")


class ImageFault(AutomationException):
    message = "Got image fault"


class ServerUnreachable(AutomationException):
    message = "The server is not reachable via the configured network"


class TearDownException(AutomationException):
    message = "%(num)d cleanUp operation failed"


class RFCViolation(RestClientException):
    message = "RFC Violation"


class FlavorNotFound(AutomationException):
    message = "Flavor not found"


class ImageNotFound(AutomationException):
    message = "Image not found"


class NetworkNotFound(AutomationException):
    message = "Network not found"


class ServerActionSucceed(AutomationException):
    message = "Instance successfully spawned."


class ServerError(AutomationException):
    message = "Instance changed into error state."


class ServerTimeOut(AutomationException):
    message = "Instance spawn timeout."


class ServerNetworkQosChangeFailed(AutomationException):
    message = "Failed to modify instance network qos."


class ServerResizeFailed(AutomationException):
    message = "Failed to resize instance type."


class ServerMigrateFailed(AutomationException):
    message = "Failed to migrate instance."


class InitFailed(AutomationException):
    message = "Failed to init test env."


class FloatingIpNotCorrectlyAssociated(AutomationException):
    message = "The floating ip is not correctly associated"


class PasswordNotCorrectlyUpdated(AutomationException):
    message = "The password is not correctly updated"


class ImageNotCorrectlyCaptured(AutomationException):
    message = "The image is not correctly captured"


class ServerNotCorrectlyCreatedFromCapturedImage(AutomationException):
    message = "The server is not correctly created from captured image"


class ServerDeleteException(AutomationException):
    message = "Failed to delete server."


class VolumeDeleteException(AutomationException):
    message = "Failed to delete volume."


class DeleteException(AutomationException):
    message = "Failed to delete resource."


class CreateMonitorException(AutomationException):
    message = "Failed to create monitor service."


class SecurityGroupException(AutomationException):
    message = "Security Group test failed"
