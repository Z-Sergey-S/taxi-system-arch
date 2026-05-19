async def test_ping(web_client):
    response = await web_client.get('/ping')
    assert response.status == 200