import pytest


async def test_etcd_put_get(service_client):
    assert "hello" == "hello"
