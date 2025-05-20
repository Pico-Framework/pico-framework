import request from 'supertest';
import assert from 'assert';

const BASE_URL = 'http://ping-a';

describe('PingPongController', function () {
  it('GET /ping should respond OK and prepare next = /pong', async () => {
    const res = await request(BASE_URL).get('/ping');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.text, 'OK');
  });

  it('GET /pong should respond OK and prepare next = /ping', async () => {
    const res = await request(BASE_URL).get('/pong');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.text, 'OK');
  });

  it('GET /log should return log text', async () => {
    const res = await request(BASE_URL).get('/log');
    assert.strictEqual(res.status, 200);
    assert.match(res.text, /PingPongController started/);
  });

  it('GET /config should return controller state', async () => {
    const res = await request(BASE_URL).get('/config');
    assert.strictEqual(res.status, 200);
    assert.ok(res.body.peerHost);
    assert.ok(res.body.nextPath);
    assert.ok('intervalMs' in res.body);
  });
});
