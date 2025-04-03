import request from 'supertest';
import { expect } from 'chai';

// Replace with your Pico's IP address or hostname
const picoUrl = 'http:192.168.50.20';

describe('Pico HTTP Server End-to-End Tests', function() {
  this.timeout(5000);

  it('GET / should return 200 OK with expected welcome text', function(done) {
    request(picoUrl)
      .get('/')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        expect(res.text).to.include('Welcome from Ian Archbell');
        done();
      });
  });

  it('GET /api with query parameters should return parsed query as JSON', function(done) {
    request(picoUrl)
      .get('/api?foo=bar&baz=qux')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        expect(res.body).to.deep.equal({ foo: 'bar', baz: 'qux' });
        done();
      });
  });

  it('POST /submit with URL-encoded form data should parse form parameters', function(done) {
    request(picoUrl)
      .post('/submit')
      .send('name=John+Doe&age=30')
      .set('Content-Type', 'application/x-www-form-urlencoded')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        expect(res.body).to.deep.equal({ name: 'John Doe', age: '30' });
        done();
      });
  });

  it('POST /upload with multipart/form-data should handle file uploads', function(done) {
    request(picoUrl)
      .post('/upload')
      .field('description', 'Test upload')
      .attach('file', 'test/fixtures/sample.txt')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        expect(res.body).to.have.property('description', 'Test upload');
        done();
      });
  });
});
