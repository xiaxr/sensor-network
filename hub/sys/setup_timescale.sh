#!/bin/bash

sudo mkdir -p /var/lib/xiaxr/data
docker run -d --name xiaxr_tsdb -p 127.0.0.1:5432:5432 -e POSTGRES_PASSWORD=ts_pwd -v /var/lib/xiaxr/data:/var/lib/postgresql/data timescale/timescaledb:2.0.0-pg12

# docker exec -it xiaxr_tsdb psql -U postgres
# CREATE database xiaxr_hub;
# \c xiaxr_hub
# CREATE EXTENSION IF NOT EXISTS timescaledb;
# CREATE TABLE measurements (
# time TIMESTAMP NOT NULL,
# device BIGINT NOT NULL,
# measurement_id INTEGER NOT NULL,
# tag TEXT NOT NULL,
# type CHAR(16) NOT NULL,
# unit CHAR(16) NOT NULL,
# value DOUBLE PRECISION NOT NULL,
# PRIMARY KEY(time, device, measurement_id)
# );
# SELECT create_hypertable('measurements', 'time', 'device', 3);
