#include <libssh/libssh.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <string.h>

int verify_knownhost(ssh_session session)
{
    enum ssh_known_hosts_e state;
    unsigned char *hash = NULL;
    ssh_key srv_pubkey = NULL;
    size_t hlen;
    char buf[10];
    char *hexa;
    char *p;
    int cmp;
    int rc;

    rc = ssh_get_server_publickey(session, &srv_pubkey);
    if (rc < 0)
    {
        return -1;
    }

    rc = ssh_get_publickey_hash(srv_pubkey,
                                SSH_PUBLICKEY_HASH_SHA1,
                                &hash,
                                &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0)
    {
        return -1;
    }

    state = ssh_session_is_known_server(session);
    switch (state)
    {
    case SSH_KNOWN_HOSTS_OK:
        /* OK */

        break;
    case SSH_KNOWN_HOSTS_CHANGED:
        fprintf(stderr, "Host key for server changed: it is now:\n");
        ssh_print_hexa("Public key hash", hash, hlen);
        fprintf(stderr, "For security reasons, connection will be stopped\n");
        ssh_clean_pubkey_hash(&hash);

        return -1;
    case SSH_KNOWN_HOSTS_OTHER:
        fprintf(stderr, "The host key for this server was not found but an other"
                        "type of key exists.\n");
        fprintf(stderr, "An attacker might change the default server key to"
                        "confuse your client into thinking the key does not exist\n");
        ssh_clean_pubkey_hash(&hash);

        return -1;
    case SSH_KNOWN_HOSTS_NOT_FOUND:
        fprintf(stderr, "Could not find known host file.\n");
        fprintf(stderr, "If you accept the host key here, the file will be"
                        "automatically created.\n");

        /* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */

    case SSH_KNOWN_HOSTS_UNKNOWN:
        hexa = ssh_get_hexa(hash, hlen);
        fprintf(stderr, "The server is unknown. Do you trust the host key?\n");
        fprintf(stderr, "Public key hash: %s\n", hexa);
        ssh_string_free_char(hexa);
        ssh_clean_pubkey_hash(&hash);
        // p = fgets(buf, sizeof(buf), stdin);
        // if (p == NULL)
        // {
        //     return -1;
        // }

        // cmp = strncasecmp(buf, "yes", 3);
        // if (cmp != 0)
        // {
        //     return -1;
        // }

        rc = ssh_session_update_known_hosts(session);
        if (rc < 0)
        {
            fprintf(stderr, "Error %s\n", strerror(errno));
            return -1;
        }

        break;
    case SSH_KNOWN_HOSTS_ERROR:
        fprintf(stderr, "Error %s", ssh_get_error(session));
        ssh_clean_pubkey_hash(&hash);
        return -1;
    }

    ssh_clean_pubkey_hash(&hash);
    return 0;
}
void read_channel(ssh_channel channel)
{
    char buffer[4096];
    int nbytes;
    // Set the channel to blocking mode
    ssh_channel_set_blocking(channel, 1);
    // Attempt to read data from the channel
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0)
    {
        std::cout.write(buffer, nbytes);
        std::cout.flush();
    }
    // Handle the case when no data is available or an error occurs
    if (nbytes == SSH_ERROR)
    {
        std::cerr << "Error reading channel: " << ssh_get_error(channel) << std::endl;
    }
}


int main()
{
    ssh_session my_ssh_session = ssh_new();
    if (my_ssh_session == nullptr)
    {
        std::cerr << "Error creating SSH session" << std::endl;
        exit(-1);
    }

    // Parse the custom SSH configuration file
    // if (ssh_options_parse_config(my_ssh_session, "config")) // Adjust the path as necessary
    // {
    //     std::cout<<"Successfully readed conf file\n";
    // }

    // Set session parameters
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "rt2.olsendata.com");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "damadaro");

    int verbosity = SSH_LOG_PROTOCOL;
    int port = 22121;
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOSTKEYS, "ssh-rsa");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HMAC_C_S, "hmac-sha1,hmac-md5");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HMAC_S_C, "hmac-sha1,hmac-md5");

    // Connect to server
    if (ssh_connect(my_ssh_session) != SSH_OK)
    {
        std::cerr << "Error connecting: " << ssh_get_error(my_ssh_session) << std::endl;
        ssh_free(my_ssh_session);
        exit(-1);
    }
    else
    {

        std::cout << "Connected...\n";
    }

    // Verify the server's identity
    // For the source code of verify_knownhost(), check previous example
    if (verify_knownhost(my_ssh_session) < 0)
    {
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        exit(-1);
    }

    // Authenticate ourselves
    char *password;
    int rc;
    rc = ssh_userauth_password(my_ssh_session, "damadaro", "clearsky2715");

    if (rc != SSH_AUTH_SUCCESS)
    {
        fprintf(stderr, "Error authenticating with password: %s\n",
                ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        exit(-1);
    }
    else
    {
        printf("AUTHENTICATED : %d\n" , rc);
    }

    

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);

    std::cout << "Disconnected." << std::endl;

    return 0;
}