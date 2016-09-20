module.exports = [
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Behaviour and look"
      },
      {
        "type": "select",
        "messageKey": "SHAKE",
        "defaultValue": 1,
        "label": "When watch is shaken",
        "options": [
          {
            "label": "Do nothing",
            "value": 0
          },
          {
            "label": "Solve expressions",
            "value": 1
          },
          {
            "label": "New expressions",
            "value": 2
          }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "LABELS",
        "defaultValue": "true",
        "label": "Show labels"
      },
      {
        "type": "toggle",
        "messageKey": "STEPS",
        "defaultValue": "false",
        "label": "Show daily step count"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Operators in expressions"
      },
      {
        "type": "select",
        "messageKey": "MODE",
        "defaultValue": "custom",
        "label": "Preset mode",
        "options": [
          {
            "label": "Beginner",
            "value": "beginner"
          },
          {
            "label": "Normal",
            "value": "normal"
          },
          {
            "label": "Hard core",
            "value": "hardcore"
          },
          {
            "label": "Custom",
            "value": "custom"
          }
        ]
      },
      {
        "type": "select",
        "messageKey": "ADD",
        "defaultValue": 3,
        "label": "Add",
        "options": [
          {
            "label": "Never",
            "value": 0
          },
          {
            "label": "Rarely",
            "value": 1
          },
          {
            "label": "Regularly",
            "value": 3
          },
          {
            "label": "Often",
            "value": 5
          }
        ]
      },
      {
        "type": "select",
        "messageKey": "SUBTRACT",
        "defaultValue": 3,
        "label": "Subtract",
        "options": [
          {
            "label": "Never",
            "value": 0
          },
          {
            "label": "Rarely",
            "value": 1
          },
          {
            "label": "Regularly",
            "value": 3
          },
          {
            "label": "Often",
            "value": 5
          }
        ]
      },
      {
        "type": "select",
        "messageKey": "MULTIPLY",
        "defaultValue": 5,
        "label": "Multiply",
        "options": [
          {
            "label": "Never",
            "value": 0
          },
          {
            "label": "Rarely",
            "value": 1
          },
          {
            "label": "Regularly",
            "value": 3
          },
          {
            "label": "Often",
            "value": 5
          }
        ]
      },
      {
        "type": "select",
        "messageKey": "DIVIDE",
        "defaultValue": 1,
        "label": "Divide",
        "options": [
          {
            "label": "Never",
            "value": 0
          },
          {
            "label": "Rarely",
            "value": 1
          },
          {
            "label": "Regularly",
            "value": 3
          },
          {
            "label": "Often",
            "value": 5
          }
        ]
      },
      {
        "type": "select",
        "messageKey": "SQUARE",
        "defaultValue": 5,
        "label": "Square",
        "options": [
          {
            "label": "Never",
            "value": 0
          },
          {
            "label": "Rarely",
            "value": 1
          },
          {
            "label": "Regularly",
            "value": 3
          },
          {
            "label": "Often",
            "value": 5
          }
        ]
      },
      {
        "type": "select",
        "messageKey": "ROOT",
        "defaultValue": 1,
        "label": "Square root",
        "options": [
          {
            "label": "Never",
            "value": 0
          },
          {
            "label": "Rarely",
            "value": 1
          },
          {
            "label": "Regularly",
            "value": 3
          },
          {
            "label": "Often",
            "value": 5
          }
        ]
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
