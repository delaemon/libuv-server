#include <iostream>
#include <uv.h>
using namespace std;

int main() {
    int r;
    uv_tcp_t tcp;
    struct sockaddr_in server_addr;

    uv_ip4_addr("127.0.0.1", 80, &server_addr);
    r = uv_tcp_init(uv_default_loop(), &tcp);
    uv_connect_t connect_req;

    r = uv_tcp_connect(&connect_req, &tcp, (struct sockaddr *)&server_addr,
        [](uv_connect_t *req, int status) {
        std::cout << "connected" << std::endl;

        int r;
        uv_buf_t buf[1];
        uv_stream_t* tcp = req->handle;
        char req_message[] = "GET / HTTP/1.0\r\n\r\n";

        buf[0].len = strlen(req_message);
        buf[0].base = req_message;

        uv_write_t write_req;
        r = uv_write(&write_req, tcp, buf, 1,
            [](uv_write_t *req, int status) {
            int r;
            uv_stream_t* tcp = req->handle;

            std::cout << "written" << std::endl;
            r = uv_read_start((uv_stream_t*) tcp,
                [](uv_handle_t* handle, size_t size, uv_buf_t *buf) {
                    std::cout << "alloc " << size << std::endl;
                    buf->base = (char*)malloc(size);
                    buf->len = size;
                    //return buf;
                },
                [](uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf) {
                    std::cout << "read" << std::endl;

                    if (nread < 0) {
                        if (buf->base) {
                            free(buf->base);
                        }
                        uv_close((uv_handle_t*) tcp,
                            [](uv_handle_t* handle){
                                std::cout << "closed" << std::endl;
                            });
                        return;
                    }
                    std::cout << buf->base << std::endl;
                    free(buf->base);
                    return;
                }
            );
        });
    });
    std::cout << "main" << std::endl;
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
