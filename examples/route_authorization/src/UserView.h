#pragma once

#include "UserView.h"
#include "framework/HtmlTemplateView.h"

/**
 * @brief View class for user-related HTML rendering (e.g. login).
 */
class UserView : public HtmlTemplateView
{
public:
    UserView();

private:
    static constexpr const char *login_html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    
    <head>
        <meta charset="UTF-8" />
        <title>Pico Login</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css" />
        <script src="https://cdn.jsdelivr.net/npm/crypto-js@4.1.1/crypto-js.min.js"></script>
        <style>
            body {
                font-family: 'Raleway', sans-serif;
                background: #e0e0e0;
                display: flex;
                justify-content: center;
                align-items: center;
                height: 100vh;
                margin: 0;
            }
    
            .login-card {
                background: #e0e0e0;
                padding: 2rem;
                border-radius: 20px;
                box-shadow: 9px 9px 16px #bebebe, -9px -9px 16px #ffffff;
                width: 320px;
                display: flex;
                flex-direction: column;
                gap: 1rem;
            }
    
            h2 {
                text-align: center;
                color: #333;
                margin-bottom: 1rem;
            }
    
            input, button, label {
                font-size: 1rem;
            }
    
            input {
                padding: 0.75rem;
                border: none;
                border-radius: 10px;
                background: #e0e0e0;
                box-shadow: inset 4px 4px 8px #bebebe,
                            inset -4px -4px 8px #ffffff;
            }
    
            button {
                padding: 0.75rem;
                border: none;
                border-radius: 10px;
                background: #e0e0e0;
                box-shadow: 4px 4px 8px #bebebe,
                            -4px -4px 8px #ffffff;
                cursor: pointer;
            }
    
            button:hover {
                box-shadow: inset 2px 2px 4px #bebebe,
                            inset -2px -2px 4px #ffffff;
            }
    
            .toggle-link {
                font-size: 0.9rem;
                text-align: center;
                color: #0077cc;
                cursor: pointer;
                text-decoration: underline;
            }
    
            #result {
                text-align: center;
                font-weight: bold;
                color: #333;
            }
            <style>
                .signout-container {
                margin-top: 2rem;
                text-align: center;
                }

                .signout-button {
                background: #e0e0e0;
                border: none;
                padding: 0.75rem 1.5rem;
                border-radius: 12px;
                box-shadow: 5px 5px 10px #bebebe, -5px -5px 10px #ffffff;
                font-size: 1rem;
                cursor: pointer;
                }

                .signout-button:hover {
                box-shadow: inset 5px 5px 10px #bebebe, inset -5px -5px 10px #ffffff;
                }
                </style>

        </style>
    </head>
    
    <body>
        <div class="login-card">
            <h2 id="formTitle">Login</h2>
            <input id="username" type="text" placeholder="Username" />
            <input id="password" type="password" placeholder="Password" />
            <label><input type="checkbox" id="remember" /> Remember me</label>
            <button onclick="submit()">Login</button>
            <div class="toggle-link" onclick="toggleMode()">Don't have an account? Sign up</div>
            <hr />
            <button onclick="testAuth()">Test Protected Route</button>
            
            <div id="result"></div>
            <div class="signout-container">
            <button class="signout-button" onclick="signOut()">Sign Out</button>
            </div>

        </div>
    
        <script>
            let isSignup = false;
    
            function toggleMode() {
                isSignup = !isSignup;
                document.getElementById("formTitle").textContent = isSignup ? "Sign Up" : "Login";
                document.querySelector("button").textContent = isSignup ? "Sign Up" : "Login";
                document.querySelector(".toggle-link").textContent = isSignup
                    ? "Already have an account? Log in"
                    : "Don't have an account? Sign up";
            }
    
            function getStorage() {
                return document.getElementById("remember").checked ? localStorage : sessionStorage;
            }
    
            async function submit() {
                const username = document.getElementById("username").value.trim();
                const password = document.getElementById("password").value;
                console.log("Username:", username);
                console.log("Password:", password);
                const hashed = await hash(password);
    
                const response = await fetch(isSignup ? "/signup" : "/auth", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ username, password: hashed })
                });
    
                const result = document.getElementById("result");
                if (!response.ok) {
                    result.textContent = "❌ Login or Signup failed.";
                    return;
                }
    
                const data = await response.json();
                getStorage().setItem("authToken", data.token);
                result.textContent = "✅ Success. Token saved.";
            }
    
            async function testAuth() {
                const token = localStorage.getItem("authToken") || sessionStorage.getItem("authToken");
                const response = await fetch("/api/v1/protected-data", {
                    headers: { Authorization: "Bearer " + localStorage.getItem("jwt") }
                });
    
                const resultEl = document.getElementById("result");
                if (!response.ok) {
                    resultEl.textContent = `❌ Unauthorized (${response.status})`;
                } else {
                    const data = await response.json();
                    resultEl.textContent = "✅ " + data.message;
                }t 
            }
    
            function hash(text) {
                return CryptoJS.SHA256(text).toString(CryptoJS.enc.Hex);
            }

            function signOut() {
                localStorage.removeItem("jwt");
                window.location.href = "/login";
            }

        </script>
    </body>
    </html>
    )rawliteral";
};
