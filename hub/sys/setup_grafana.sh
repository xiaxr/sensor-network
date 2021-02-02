#!/bin/bash

sudo mkdir -p /var/lib/xiaxr/graf

# docker run -ti --user root -v /var/lib/xiaxr/graf:/var/lib/grafana --entrypoint bash grafana/grafana
# chown -R root:root /etc/grafana && chmod -R a+r /etc/grafana && chown -R grafana /var/lib/grafana && chown -R grafana /usr/share/grafana

docker run -d --name xiaxr_graf --network=host -v /var/lib/xiaxr/graf:/var/lib/grafana grafana/grafana

# pwd VVYCAkXbgG878P5