import datetime
import peewee as pw

from .config import DATABASE_PATH
from .device import generate_device_id

device_db = pw.SqliteDatabase(DATABASE_PATH)


class BaseModel(pw.Model):
    class Meta:
        database = device_db


class DeviceEntry(BaseModel):
    device_id = pw.FixedCharField(max_length=32, unique=True, null=False)
    device_node_id = pw.IntegerField()
    device_name = pw.CharField(max_length=64)
    device_type = pw.IntegerField()
    device_description = pw.TextField()
    active_last = pw.DateTimeField(null=True)


class MeasurementEntry(BaseModel):
    measurement_id = pw.BigIntegerField(unique=True, null=False)
    device = pw.ForeignKeyField(DeviceEntry,
                                backref='measurements',
                                null=False)
    measurement_type = pw.IntegerField(null=False)
    measurement_description = pw.TextField()


# todo place in a queue
class MeasurementValue(BaseModel):
    measurement = pw.ForeignKeyField(MeasurementEntry,
                                     backref='values',
                                     null=False)
    value = pw.DoubleField(null=False)
    timestamp = pw.DateTimeField(default=datetime.datetime.utcnow)


def init_db():
    with device_db:
        device_db.create_tables(
            [DeviceEntry, MeasurementEntry, MeasurementValue])


def _update_master(device_id, name, description):
    with device_db:
        # expunge any old masters
        query = DeviceEntry.delete().where(DeviceEntry.device_node_id == 0)
        query.execute()

        # check for duplicate ids and addresses
        while DeviceEntry.select().where(
                DeviceEntry.device_id == device_id).count() > 0:
            device_id = generate_device_id()

        DeviceEntry(device_id=device_id,
                    device_node_id=0,
                    device_name=name,
                    device_description=description,
                    active_last=datetime.datetime.utcnow())
        return device_id

def _new_generic_device():
    device_id = generate_device_id()
    with device_db:
        while DeviceEntry.select().where(
                DeviceEntry.device_id == device_id).count() > 0:
            device_id = generate_device_id()
        
        DeviceEntry(device_id=device_id,
            device_node_id=0xFFFF,
            device_name="generic",
            device_description="",
            active_last=datetime.datetime.utcnow())
        
        return device_id

def new_device_entry(device_id, node_id, name, description):
    if node_id == 0:
        return _update_master(device_id, name, description)

    if node_id == 0xFFFF:
        return _new_generic_device()


    return device_id
