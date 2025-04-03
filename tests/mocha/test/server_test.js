import request from 'supertest';
import { expect } from 'chai';

// Change this to the correct IP/hostname and port where your test firmware is running.
const picoUrl = 'http://192.168.50.20';

describe('HTTP Server Routing Tests', function() {
  // Increase timeout if your device is slow to respond.
  this.timeout(5000);

  // --- Static Route Test ---
  it('GET / should return "Hello from Ian Archbell!"', function(done) {
    request(picoUrl)
      .get('/')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        expect(res.text).to.contain('Hello from Ian Archbell!');
        done();
      });
  });

  // --- Dynamic Route Test ---
  // For this test, ensure your test firmware registers a route like:
  // router.addRoute("GET", "/user/{id}", (req, res, params) => {
  //     res.json({ id: params[0] });
  // });
  it('GET /user/:id should extract dynamic parameter', function(done) {
    request(picoUrl)
      .get('/user/12345')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        // Expect the route handler to return a JSON object with the id extracted from the URL.
        expect(res.body).to.have.property('id', '12345');
        done();
      });
  });

  // --- Protected Route Test Without Auth ---
  // For this test, ensure your test firmware defines a route (e.g., /auth) that requires authorization.
  // If no valid Authorization header is provided, the route should return 401 Unauthorized.
  it('GET /auth without auth header should return 401 Unauthorized', function(done) {
    request(picoUrl)
      .get('/auth')
      .expect(401)
      .end(function(err, res) {
        if (err) return done(err);
        expect(res.text).to.contain('Unauthorized');
        done();
      });
  });

  // --- Protected Route Test With Auth ---
  // For this test, your test firmware should validate the token (for instance, "validToken123") and return a JSON object.
  it('GET /auth with valid auth header should return token', function(done) {
    request(picoUrl)
      .get('/auth')
      .set('Authorization', 'Bearer validToken123')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        // Expect the response to include a token field (or similar indication of successful auth).
        expect(res.body).to.have.property('token');
        done();
      });
  });

  // --- Catch-all Route Test ---
  // If your Router defines a catch-all route for static files (e.g. GET "/(.*)"),
  // this test verifies that a request to a non-existent file returns some content.
  it('GET /nonexistent should return static content from catch-all route', function(done) {
    request(picoUrl)
      .get('/nonexistent')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        // You might expect a 404 page or some default static page content.
        expect(res.text).to.not.be.empty;
        done();
      });
  });
});
