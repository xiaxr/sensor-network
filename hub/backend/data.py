import datetime
import os

import peewee as pw

_DB_DIR = _SETTINGS_DIR = os.path.dirname(__file__)

device_db = pw.SqliteDatabase(os.path.join(_DB_DIR, "devices.db"))


class BaseModel(pw.Model):
    class Meta:
        database = device_db


class Device(BaseModel):
    device_id = pw.FixedCharField(max_length=32, unique=True, null=False)
    device_address = pw.FixedCharField(max_length=40, null=False)
    device_name = pw.CharField(max_length=64)
    device_description = pw.TextField()
    is_master = pw.BooleanField(null=False)
    active_last = pw.DateTimeField(null=True)


class Measurement(BaseModel):
    measurement_id = pw.BigIntegerField(unique=True, null=False)
    device = pw.ForeignKeyField(Device, backref='measurements', null=False)
    measurement_type = pw.IntegerField(null=False)
    measurement_description = pw.TextField()


# todo place in a queue
class MeasurementValue(BaseModel):
    measurement = pw.ForeignKeyField(Measurement, backref='values', null=False)
    value = pw.DoubleField(null=False)
    timestamp = pw.DateTimeField(default=datetime.datetime.utcnow)


def init_db():
    with device_db:
        device_db.create_tables([Device, Measurement, MeasurementValue])


def update_master(gateway):
    with device_db:
        # expunge any old masters
        query = Device.delete().where(Device.is_master)
        query.execute()

        # check for duplicate ids and addresses

        while Device.select().where(
                Device.device_id == gateway.gateway_id).count() > 0:
            gateway.regenerate_gateway_id()

        Device(device_id=gateway.gateway_id,
               device_address=gateway.gateway_address,
               device_name="xiaxr hub",
               device_description="xiaxr master sensor hub",
               is_master=True,
               active_last=datetime.datetime.utcnow())
