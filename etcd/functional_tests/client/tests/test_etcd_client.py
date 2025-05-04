import pytest


async def test_etcd_put_get(service_client, etcd_mock):
    response = await service_client.post(
        '/v1/get',
        params={'key': 'some_key'}
    )
    assert response.status == 200
    assert response.content == b'No value'
    
    response = await service_client.put(
        '/v1/put',
        params={'key': 'some_key', 'value': 'some_value'}
    )
    assert response.status == 200

    response = await service_client.post(
        '/v1/get',
        params={'key': 'some_key'}
    )
    assert response.status == 200
    assert response.content == b'some_value'
