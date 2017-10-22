# Query 2 Path

Query 2 Path is a so-called Store-Id program for the Squid proxy server. Store-Id programs re-write URLs before they are cached (or not). Query 2 Path is designed to deal with URLs that cannot normally be cached because the relevant resource identifiers are hidden as query parameters.

URLs of that nature are broken by design and not REST compliant. Query 2 Path comes to the rescue by turning query parameters into path segments.

Query path is written in C, does not spawn processes and does not allocate memory at runtime. Hence it is super efficient. Normally you would not need multiple instances even for a busy Squid installation.

## Restrictions

Unfortunately, not all query parameters are identifiers (but, for instance, formatting options). Another restriction is dynamic resources that are called frequently with different query parameters each time. Caching those would waste cache memory.

Hence, every target domain needs to be carefully analysed as to whether Query 2 Path should be applied or not.

## Build

Change to the source directory and issue the following command:

	gcc -O3 -o query2path query2path.c

Afterwards you might want to copy the query2path binary to a standard directory like /usr/local/bin.

## Installation

Identify the target domains and enable query2path in your squid configuration. To that end you would probably add options similar to the following to your squid.conf file:

	acl rewritedoms dstdomain .subdomain.domain.com
	store_id_program /usr/local/bin/query2path
	store_id_children 1 startup=1 idle=1 concurrency=0
	store_id_access allow rewritedoms
	store_id_access deny all

NB: whilst unrelated to the actual query2path program, you might want to tweak the cache policies along the lines of

	refresh_pattern ^http:.*\.domain\.com 5256000 20% 5256000 override-lastmod override-expire ignore-no-store ignore-must-revalidate ignore-reload

## Testing

Since a Store-Id program is a simple filter from stdin to stdout you can run a simple unit test by piping testcases into it:

	./query2path < testcases.txt

For testing query2path in the context of Squid you need to build a debug version by setting the correponding pre-processor variable. Query2path will then log its activites to a file that is also defined in the coding.

