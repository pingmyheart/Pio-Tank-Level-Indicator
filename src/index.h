#pragma once
#include <WString.h>

inline String indexHtml=R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Water Level</title>

    <style>
        body {
            margin: 0;
            font-family: Arial, sans-serif;
            background: radial-gradient(circle at top, #1e3c72, #0b1a2e);
            color: white;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        .tank {
            width: 220px;
            height: 320px;
            border: 5px solid rgba(255,255,255,0.5);
            border-radius: 20px;
            position: relative;
            overflow: hidden;
            background: rgba(255,255,255,0.05);
        }

        .water {
            position: absolute;
            bottom: 0;
            width: 100%;
            height: %water%;
            background: linear-gradient(
                    180deg,
                    #4fc3f7,
                    #0288d1
            );
        }

        /* fake wave */
        .water::before {
            content: "";
            position: absolute;
            top: -18px;
            left: -10%;
            width: 120%;
            height: 36px;
            background: rgba(255,255,255,0.35);
            border-radius: 50%;
        }

        .label {
            position: absolute;
            inset: 0;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 2.2rem;
            font-weight: bold;
            text-shadow: 0 2px 5px rgba(0,0,0,0.7);
            pointer-events: none;
        }

        .footer {
            position: absolute;
            bottom: -45px;
            width: 100%;
            text-align: center;
            font-size: 0.9rem;
            opacity: 0.8;
        }
    </style>
</head>

<body>
<div class="tank">
    <div class="water"></div>
    <div class="label">%water%</div>
    <div class="footer">Water Level</div>
</div>
</body>
</html>
)rawliteral";