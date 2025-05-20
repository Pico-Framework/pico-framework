// test/tls.test.js
import request from 'supertest';
import assert from 'assert';

const BASE_URL = 'http://pico-framework';
const TEST_FILE_PATH = 'test_tls_output.txt';

describe('TLS Route Tests', function () {
  this.timeout(10000); // Allow extra time for network TLS fetch

  it('GET /tls should return a JSON body with the TLS response text', async () => {
    const res = await request(BASE_URL).get('/tls');
    assert.strictEqual(res.status, 200);
    assert.ok(res.body.Response, 'Missing Response field');
    assert.ok(typeof res.body.Response === 'string');
    assert.ok(res.body.Response.length > 0, 'Empty TLS response');
  });

  it(`GET /tlstofile/${TEST_FILE_PATH} should save and return the file`, async () => {
    const res = await request(BASE_URL).get(`/tlstofile/${encodeURIComponent(TEST_FILE_PATH)}`);
    assert.strictEqual(res.status, 200);
    assert.ok(res.text.length > 0, 'Expected file content in response');
    assert.ok(res.text.startsWith('9f26719cc254d701ccc1ae654649e31db3f033f13984dc48bc3c0d9dfc12fe77'), 'Unexpected file prefix');
  });

  it(`GET /ls should confirm ${TEST_FILE_PATH} exists`, async () => {
    const res = await request(BASE_URL).get('/ls');
    assert.strictEqual(res.status, 200);
    const files = res.body.map(f => f.name);
    assert.ok(files.includes(TEST_FILE_PATH), `Expected ${TEST_FILE_PATH} in ls`);
  });
});
