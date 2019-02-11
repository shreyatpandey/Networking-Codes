struct msghdr mhdr;
struct iovec iov[1];
struct cmsghdr *cmhdr;
char control[1000];
struct sockaddr_in sin;
char databuf[1500];
unsigned char tos;

mhdr.msg_name = &sin
mhdr.msg_namelen = sizeof(sin);
mhdr.msg_iov = iov;
mhdr.msg_iovlen = 1;
mhdr.msg_control = &control;
mhdr.msg_controllen = sizeof(control);
iov[0].iov_base = databuf;
iov[0].iov_len = sizeof(databuf);
memset(databuf, 0, sizeof(databuf));
if ((*len = recvmsg(sock, &mhdr, 0)) == -1) {
    perror("error on recvmsg");
    exit(1);
} else {
    cmhdr = CMSG_FIRSTHDR(&mhdr);
    while (cmhdr) {
        if (cmhdr->cmsg_level == IPPROTO_IP && cmhdr->cmsg_type == IP_TOS) {
            // read the TOS byte in the IP header
            tos = ((unsigned char *)CMSG_DATA(cmhdr))[0];
        }
        cmhdr = CMSG_NXTHDR(&mhdr, cmhdr);
    }
    printf("data read: %s, tos byte = %02X\n", databuf, tos); 
}
