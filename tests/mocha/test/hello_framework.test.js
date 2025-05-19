
import request from 'supertest';
import assert from 'assert';

const baseUrl = 'http://Pico-Framework';

describe('hello_framework Regression Tests', function () {
  this.timeout(10000);

  afterEach(function (done) {
    setTimeout(done, 100);
  });

  it('GET / should return greeting', async function () {
    const res = await request(baseUrl).get('/');
    assert.strictEqual(res.statusCode, 200);
    assert.match(res.text, /Hello from Ian Archbell!/);
  });

  it('GET /name/John should greet John', async function () {
    const res = await request(baseUrl).get('/name/John');
    assert.strictEqual(res.statusCode, 200);
    assert.match(res.text, /Hello John!/);
  });

  it('GET /api/data should return mocked JSON', async function () {
    const res = await request(baseUrl).get('/api/data');
    assert.strictEqual(res.statusCode, 200);
    assert.strictEqual(res.body.key, 'value');
    assert.strictEqual(res.body.number, 42);
  });

  it('POST /submit should echo raw body', async function () {
    const res = await request(baseUrl).post('/submit').send('my raw data');
    assert.strictEqual(res.statusCode, 200);
    assert.match(res.text, /Data received: my raw data/);
  });

  it('POST /api/json with valid JSON should echo it', async function () {
    const body = { key: "value", number: 123 };
    const res = await request(baseUrl)
      .post('/api/json')
      .send(body)
      .set('Content-Type', 'application/json');
    assert.strictEqual(res.statusCode, 200);
    assert.deepStrictEqual(res.body.received, body);
  });

  it('POST /api/json with invalid JSON should return 400', async function () {
    const res = await request(baseUrl)
      .post('/api/json')
      .send("{ invalid }")
      .set('Content-Type', 'application/json');
    assert.strictEqual(res.statusCode, 400);
    assert.strictEqual(res.body.error.code, "INVALID_JSON");
  });

  it('POST /api/form should return parsed form data', async function () {
    const res = await request(baseUrl)
      .post('/api/form')
      .send('name=ian&role=tester')
      .set('Content-Type', 'application/x-www-form-urlencoded');
    assert.strictEqual(res.statusCode, 200);
    assert.deepStrictEqual(res.body.received, { name: 'ian', role: 'tester' });
  });

  it('GET /api/query with parameters should return them', async function () {
    const res = await request(baseUrl).get('/api/query?foo=bar&baz=qux');
    assert.strictEqual(res.statusCode, 200);
    assert.deepStrictEqual(res.body.received, { foo: 'bar', baz: 'qux' });
  });

  it('PUT /update/123 should acknowledge update', async function () {
    const res = await request(baseUrl).put('/update/123');
    assert.strictEqual(res.statusCode, 200);
    assert.match(res.text, /PUT request for ID: 123/);
  });

  it('DELETE /delete/456 should acknowledge deletion', async function () {
    const res = await request(baseUrl).delete('/delete/456');
    assert.strictEqual(res.statusCode, 200);
    assert.match(res.text, /DELETE request for ID: 456/);
  });

  it('GET /api/header should return User-Agent', async function () {
    const res = await request(baseUrl).get('/api/header').set('User-Agent', 'MochaTest');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(res.body["user-agent"]);
  });

  it('GET /api/headers should include all headers', async function () {
    const res = await request(baseUrl).get('/api/headers');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(res.body["all-headers"]);
    assert.ok(res.body["user-agent"]);
  });

  it('GET /api/custom should return custom header', async function () {
    const res = await request(baseUrl).get('/api/custom');
    assert.strictEqual(res.statusCode, 202);
    assert.strictEqual(res.headers['x-custom-header'], 'PicoFramework');
    assert.strictEqual(res.text, 'Accepted but not processed');
  });

  it('HEAD /status should return headers without body', async function () {
    const res = await request(baseUrl).head('/status');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(res.headers['x-framework']);
  });

  it('OPTIONS /api/data should return allowed methods', async function () {
    const res = await request(baseUrl).options('/api/data');
    assert.strictEqual(res.statusCode, 204);
    assert.strictEqual(res.headers['allow'], 'GET,POST,OPTIONS');
  });

  it('GET /not-found should return 404', async function () {
    const res = await request(baseUrl).get('/not-found');
    assert.strictEqual(res.statusCode, 404);
    assert.match(res.text, /Not Found/);
  });
});
