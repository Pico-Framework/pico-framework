/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include <stdio.h>
 #include <string.h>
 #include "pico/async_context.h"
 #include "lwip/altcp.h"
 #include "lwip/altcp_tls.h"
 #include "http_client_util.h"
 
 #ifndef HTTP_INFO
 #define HTTP_INFO printf
 #endif
 
 #ifndef HTTP_INFOC
 #define HTTP_INFOC putchar
 #endif
 
 #ifndef HTTP_INFOC
 #define HTTP_INFOC putchar
 #endif
 
 #ifndef HTTP_DEBUG
 #ifdef NDEBUG
 #define HTTP_DEBUG
 #else
 #define HTTP_DEBUG printf
 #endif
 #endif
 
 #ifndef HTTP_ERROR
 #define HTTP_ERROR printf
 #endif

int body_state = 0;
int buffer_state = 0;

char buffer[4096];
int content_length = 0;
int total_received = 0;
int content_length_remaining = 0;
int chunk_length = 0;
int chunk_offset = 0;
int chunk_remaining = 0;
char* ptr = buffer;
int buffer_count = 0;
char header_buffer[512];

 enum body_state {
    BODY_STATE_NONE,
    BODY_STATE_CHUNKED,
    BODY_STATE_CONTENT_LENGTH
};

enum buffer_state {
    BUFFER_STATE_NONE,
    BUFFER_STATE_CHUNK_START,
    BUFFER_STATE_READ,
    BUFFER_STATE_CHUNK_END,
    BUFFER_STATE_CHUNK_DONE,   
    BUFFER_STATE_DONE     
};

bool first_buffer = false;

void parse_headers(char* headers) {
    char* line = strtok(headers, "\r\n");
    while (line != NULL) {
        printf("Header: %s\n", line);
        
        // Check for Content-Length
        if (strstr(line, "Content-Length:") != NULL) {
            char* content_length_str = strchr(line, ':') + 1;
            content_length = atoi(content_length_str);
            body_state = BODY_STATE_CONTENT_LENGTH;
            buffer_state = BUFFER_STATE_READ;
            printf("Content-Length: %d\n", content_length);
        }
        
        // Move to the next line
        line = strtok(NULL, "\r\n");
        
        // Ensure line is not NULL before proceeding with further checks
        if (line == NULL) break;
    
        // Check for Transfer-Encoding: chunked
        if (strstr(line, "Transfer-Encoding: chunked") != NULL) {
            printf("Transfer-Encoding: chunked\n");
            body_state = BODY_STATE_CHUNKED;
            buffer_state = BUFFER_STATE_CHUNK_START;
            first_buffer = true;
        }
        
        // Check for Content-Type
        if (strstr(line, "Content-Type:") != NULL) {
            char* content_type_str = strchr(line, ':') + 1;
            printf("Content-Type: %s\n", content_type_str);
        }
    }
}
 
 // Print headers to stdout
 err_t http_client_header_print_fn(__unused httpc_state_t *connection, __unused void *arg, struct pbuf *hdr, u16_t hdr_len, __unused u32_t content_len) {
    HTTP_INFO("\nheaders %u\n", hdr_len);
    u16_t offset = 0;
    char* ptr = header_buffer;
    while (offset < hdr->tot_len && offset < hdr_len) {
        char c = (char)pbuf_get_at(hdr, offset++);
        *ptr++ = c;
        HTTP_INFOC(c);
    }
    *ptr = '\0';
    parse_headers(header_buffer);
    return ERR_OK;
 }
 
 int find_crlf(const char *str) {
    const char *crlf = "\r\n";
    if (str == NULL || crlf == NULL) {
        return -1;
    }
    return (strstr(str, crlf) != NULL) ? strstr(str, crlf) - str : -1;
}

 // Print body to stdout
 err_t http_client_receive_print_fn(__unused void *arg, __unused struct altcp_pcb *conn, struct pbuf *p, err_t err) {


    if (body_state == BODY_STATE_NONE) {
        printf("No body state set\n");
        return ERR_OK;
    }

    if (body_state == BODY_STATE_CONTENT_LENGTH) {
        printf("Content-Length body state\n");
        switch (buffer_state) {
            case BUFFER_STATE_READ:
                printf("Handling buffer read\n");
    
                // If content_length is zero or not set yet, find the content length from the response
                if (content_length == 0) {
                    // Parse content length from HTTP response headers (assume it's available)
                    // This part assumes you've already parsed the headers to get the Content-Length
                    // If you don't already have it, you would need logic to extract it from the HTTP header
                    content_length = 1024;  // Example Content-Length (e.g., from headers)
    
                    printf("Content-Length: %d\n", content_length);
                }
    
                // Calculate how much data to copy
                int bytes_to_copy = content_length - total_received;
                if (bytes_to_copy > p->tot_len) {
                    bytes_to_copy = p->tot_len;
                }
    
                // Copy the data into the buffer
                memcpy(buffer + total_received, p->payload, bytes_to_copy);
                total_received += bytes_to_copy;
    
                printf("Bytes copied: %d\n", bytes_to_copy);
                printf("Buffer count: %d\n", total_received);
                printf("Buffer: %.*s\n", total_received, buffer);
    
                // Check if the full content length has been received
                if (total_received == content_length) {
                    printf("Full content received.\n");
                    buffer_state = BUFFER_STATE_DONE; // Mark as done when full content is received
                }
    
                break;
    
            case BUFFER_STATE_DONE:
                printf("Handling buffer done - won't come through here as there's no more buffers\n");
                // All data has been received
                // You can process the complete data in the buffer here, for example:
                printf("Final buffer content: %.*s\n", total_received, buffer);
                break;
    
            default:
                printf("Unexpected buffer state\n");
                break;
        }
        return ERR_OK;
    }
        
    if (body_state == BODY_STATE_CHUNKED) {
        printf("Chunked body state\n");

        switch (buffer_state) {
            case BUFFER_STATE_CHUNK_START:
                printf("Handling chunk start\n");
                int crlf_offset = find_crlf((const char *)p->payload);
        
                if (crlf_offset != -1) {
                    printf("CRLF found at %i\n", crlf_offset);
                    chunk_length = strtol((const char *)p->payload, NULL, 16);  // Parse chunk size
                    printf("chunk length: %i\n", chunk_length);
                    chunk_remaining = chunk_length;
                    printf("chunk remaining: %i\n", chunk_remaining);
        
                    // Calculate the number of bytes to copy (skip chunk length and CRLF)
                    int bytes_to_copy = chunk_remaining < p->tot_len ? chunk_remaining : p->tot_len;
                    if (first_buffer) {
                        bytes_to_copy -= crlf_offset + 2;  // Skip the chunk length and CRLF - it seems like this needs to be true for first buffer but not last
                        first_buffer = false;
                    }
                         
                    printf("bytes to copy: %i\n", bytes_to_copy);
                    printf("buffer count: %i\n", buffer_count);
                    
                    // Ensure we don't copy more than the available data
                    bytes_to_copy = bytes_to_copy < 0 ? 0 : bytes_to_copy;
                    
                    // Copy the chunk data into the buffer
                    memcpy(buffer + buffer_count, p->payload + crlf_offset + 2, bytes_to_copy);
                    buffer_count += bytes_to_copy;
                    chunk_remaining -= bytes_to_copy;
        
                    printf("chunk remaining: %i\n", chunk_remaining);
                    printf("buffer count: %i\n", buffer_count);
                    printf("buffer: %.*s\n", buffer_count, buffer);
        
                    // Check if the chunk is complete, transition to the next state
                    if (chunk_remaining == 0) {
                        printf("chunk complete\n");
                        buffer_state = BUFFER_STATE_CHUNK_START;  // Look for next chunk
                    } else {
                        buffer_state = BUFFER_STATE_READ;  // Continue reading data
                    }
                } else {
                    printf("No CRLF found\n");
                    buffer_state = BUFFER_STATE_CHUNK_END;  // End chunk if CRLF is missing
                }
        
                // If chunk length is 0, transition to chunk end
                if (chunk_length == 0) {
                    buffer_state = BUFFER_STATE_CHUNK_END;
                }
                break;
        
            case BUFFER_STATE_READ:
                printf("Handling buffer read\n");
        
                if (chunk_remaining > 0) {
                    int bytes_to_copy = chunk_remaining < p->tot_len ? chunk_remaining : p->tot_len;
                    printf("bytes to copy: %i\n", bytes_to_copy);
                    printf("buffer count: %i\n", buffer_count);
                    printf("chunk remaining: %i\n", chunk_remaining);
                    
                    // Ensure we copy the remaining data into the buffer
                    memcpy(buffer + buffer_count, p->payload, bytes_to_copy);
                    buffer_count += bytes_to_copy;
                    chunk_remaining -= bytes_to_copy;
        
                    printf("chunk remaining: %i\n", chunk_remaining);
                    printf("buffer count: %i\n", buffer_count);
                    printf("buffer: %.*s\n", buffer_count, buffer);
                }
        
                // Transition to chunk start when the chunk is completely processed
                if (chunk_remaining == 0) {
                    printf("chunk complete\n");
                    buffer_state = BUFFER_STATE_CHUNK_START;  // Look for next chunk
                }
        
                // Handle case where no more data is available
                if (chunk_remaining == 0 && p->tot_len == 0) {
                    printf("chunk complete and no more data\n");
                    buffer_state = BUFFER_STATE_CHUNK_END;  // End the chunk processing
                }
                break;
        
            case BUFFER_STATE_CHUNK_END:
                printf("Handling chunk end\n");
                // If chunk_length is zero, this indicates the end of the chunks
                if (chunk_length == 0) {
                    printf("Last chunk (size 0)\n");
                    buffer_state = BUFFER_STATE_CHUNK_END;  // End processing
                }
                break;
        
            default:
                printf("Handling default buffer\n");
                buffer_count = 0;  // Reset buffer count for unexpected cases
                break;
        }
    }
    return ERR_OK;
 }

 static err_t internal_header_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len) {
     assert(arg);
     HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
     if (req->headers_fn) {
         return req->headers_fn(connection, req->callback_arg, hdr, hdr_len, content_len);
     }
     return ERR_OK;
 }
 
 static err_t internal_recv_fn(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err) {
     assert(arg);
     HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
     if (req->recv_fn) {
         return req->recv_fn(req->callback_arg, conn, p, err);
     }
     return ERR_OK;
 }
 
 static void internal_result_fn(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) {
     assert(arg);

     HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
     HTTP_DEBUG("result %d len %u server_response %u err %d\n", httpc_result, rx_content_len, srv_res, err);
     req->complete = true;
     req->result = httpc_result;
     if (req->result_fn) {
         req->result_fn(req->callback_arg, httpc_result, rx_content_len, srv_res, err);
     }
 }
 
 // Override altcp_tls_alloc to set sni
 static struct altcp_pcb *altcp_tls_alloc_sni(void *arg, u8_t ip_type) {
     assert(arg);
     HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
     struct altcp_pcb *pcb = altcp_tls_alloc(req->tls_config, ip_type);
     if (!pcb) {
         HTTP_ERROR("Failed to allocate PCB\n");
         return NULL;
     }
     mbedtls_ssl_set_hostname(altcp_tls_context(pcb), req->hostname);
     return pcb;
 }
 
 // Make a http request, complete when req->complete returns true
 int http_client_request_async(async_context_t *context, HTTP_REQUEST_T *req) {
 #if LWIP_ALTCP
     const uint16_t default_port = req->tls_config ? 443 : 80;
     if (req->tls_config) {
         if (!req->tls_allocator.alloc) {
             req->tls_allocator.alloc = altcp_tls_alloc_sni;
             req->tls_allocator.arg = req;
         }
         req->settings.altcp_allocator = &req->tls_allocator;
     }
 #else
     const uint16_t default_port = 80;
 #endif
     req->complete = false;
     req->settings.headers_done_fn = req->headers_fn ? internal_header_fn : NULL;
     req->settings.result_fn = internal_result_fn;
     async_context_acquire_lock_blocking(context);
     err_t ret = httpc_get_file_dns(req->hostname, req->port ? req->port : default_port, req->url, &req->settings, internal_recv_fn, req, NULL);
     async_context_release_lock(context);
     if (ret != ERR_OK) {
         HTTP_ERROR("http request failed: %d", ret);
     }
     return ret;
 }
 
 // Make a http request and only return when it has completed. Returns true on success
 int http_client_request_sync(async_context_t *context, HTTP_REQUEST_T *req) {
     assert(req);
     int ret = http_client_request_async(context, req);
     if (ret != 0) {
         return ret;
     }
     while(!req->complete) {
         async_context_poll(context);
         async_context_wait_for_work_ms(context, 1000);
     }
     return req->result;
 }