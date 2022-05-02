const renderLoading = root => {
  root.innerHTML = `<div id="loading">loading...</div>`;
};

/* Store best score to localStorage and map */
const updateBestScore = (levelNumber, score) => {
  localStorage.setItem(levelNumber, score);
};

const renderMenu = root => {
  const menuHTML = `
  <div id="menu">
    <h1>Sokoban</h1>
    <div class="attribution">
      <em>by Irvin, Juan, Severin & Greg</em>
    </div>
    <ul>
      ${[...Array(5/* TODO get levels from soko */)].map((_, i) => `
        <li>
          <a href="#${i + 1}">
            <span class="material-symbols-outlined">
              ${(localStorage.getItem(i + 1) == null) ? "check_box_outline_blank" : "check_box"}
            </span>
            <span>level ${i + 1}</span>
            <span>best: ${(localStorage.getItem(i + 1) == null)  ? "&infin;" : localStorage.getItem(i + 1)}</span>
          </a>
        </li>
      `).join("")}
    </ul>
  </div>
  `;
  root.innerHTML = menuHTML;
};

const root = document.getElementById("app");
renderLoading(root);

const handleRouting = () => {
  const level = +location.hash.replace(/^#/, "") - 1;

  if (level >= 0) {
    renderLevel(root, level);
  }
  else {
    renderMenu(root);
  }
};
window.onhashchange = handleRouting;

const onLoaded = () => {
  handleRouting();
};

const renderLevel = (root, levelNumber) => {
  const gameHTML = `
  <div id="complete-level-screen"></div>
  <div id="game" class="hide">
    <div id="controls">
      <button title="Undo (z key)" id="undo"><span class="material-symbols-outlined">undo</span></button>
      <button title="Reset (r key)" id="reset"><span class="material-symbols-outlined">refresh</span></button>
      <button title="Home" id="change-level"><span class="material-symbols-outlined">home</span></button>
      <button title="Settings" disabled><span class="material-symbols-outlined">settings</span></button>
      <button title="Help" disabled><span class="material-symbols-outlined">help</span></button>
    </div>
    <div id="board"></div>
    <div id="status"></div>
  </div>
  `;
  root.innerHTML = gameHTML;
  document
    .querySelector("#change-level")
    .addEventListener("click", () => {
      location.hash = "";
      renderMenu(root);
    })
  ;
  Module.ccall("sokoban_initialize");
  const soko = {
    levelNumber: Module.cwrap("sokoban_level"),
    levelsSize: Module.cwrap("sokoban_levels_size"),
    movesSize: Module.cwrap("sokoban_moves_size"),
    move: Module.cwrap(
      "sokoban_move", // name of C function
      "bool",         // return type
      ["string"],     // argument types
    ),
    goto: Module.cwrap(
      "sokoban_goto",
      "bool",
      ["unsigned int",
      "unsigned int"]
    ),
    boardToStr: Module.cwrap(
      "sokoban_board_to_string",
      "string", // return type
    ),
    sequence: Module.cwrap(
      "sokoban_sequence",
      "string", // return type
    ),
    reset: Module.cwrap("sokoban_reset"),
    undo: Module.cwrap("sokoban_undo", "bool"),
    solved: Module.cwrap("sokoban_solved", "bool"),
    changeLevel: Module.cwrap(
      "sokoban_change_level",
      "bool",
      ["number"]
    ),
  };

  if (levelNumber < 0 || levelNumber > soko.levelsSize()) {
    location.hash = "";
    return;
  }

  soko.changeLevel(levelNumber);

  const boardEl = document.getElementById("board");
  const undoEl = document.getElementById("undo");
  const resetEl = document.getElementById("reset");
  const cellToClass = {
    "_": "floor-outside",
    " ": "floor",
    "#": "wall",
    "@": "player",
    "+": "player-on-goal",
    "$": "box",
    "*": "box-on-goal",
    ".": "goal",
  };
  const buildRowHTML = (row, rowIndex) => `
    <tr>
      ${[...row.replace(/(?:^ +)|(?: +$)/g, m => "_".repeat(m.length))]
        .map((cell, i) => `
          <td
            data-row="${rowIndex}"
            data-col="${i}"
            class="cell ${cellToClass[cell] || ""}"
          ></td>
        `)
        .join("")}
    </tr>
  `;

  const renderLevelComplete = () => {
    const completeHTML = `
    <div id="modal">
      <div class="modal-content"> 
        <h1 class="text-glow">Level ${soko.levelNumber()+1}</h>
        <h1 class="text-glow">Complete!</h1>
        <table>
          <tr>
            <td>Current moves:</td>
            <td id="current-move" class="score">${soko.movesSize()}</td>
          </tr>
          <tr>
            <td>Best moves:</td>
            <td id="best-move" class="score">${localStorage.getItem(soko.levelNumber()+1)}
            </td>
          </tr>
        </table>
        <div id="controls">
          <button title="Random" id="random-level"><span class="material-symbols-outlined">shuffle</span></button>
          <button title="Home" id="change-level"><span class="material-symbols-outlined">home</span></button>
          <button title="Next" id="next"><span class="material-symbols-outlined">fast_forward</span></button>
        </div>
    </div>
    `;
    document.querySelector("#complete-level-screen").innerHTML = completeHTML;

    /* Listens to home selector */
    document.querySelector("#change-level").addEventListener("click", () => {
      location.hash = "";
      renderMenu(root);
    })
    ;

    /* Listens to next level selector */
    document.querySelector("#next").addEventListener("click", () => {
      if (levelNumber < (soko.levelsSize() - 1)) {
        location.hash = (levelNumber + 2).toString();
        renderLevel(root, levelNumber + 1);
      }
    })
    ;

    /* Listens to random level selector */
    document.querySelector("#random-level").addEventListener("click", () => {
      const activeLevels = Object.keys(localStorage);
      const randomLevelNumber = Math.floor(Math.random() * soko.levelsSize());
      // TODO: Needs work
      // while (activeLevels.includes(randomLevelNumber+1)) {
      //   randomLevelNumber = Math.floor(Math.random() * soko.levelsSize());
      // }
      location.hash = (randomLevelNumber+1).toString();
      renderLevel(root, randomLevelNumber);
    })
    ;
  };

  const renderBoard = () => {
    boardEl.innerHTML =
      "<table><tbody>" +
        soko.boardToStr()
          .split("\n")
          .map(buildRowHTML)
          .join("") +
      "</tbody></table>"
    ;
    renderStatus();
  };
  boardEl.addEventListener("click", event => {
    const cell = event.target.closest("td");

    if (!cell || !boardEl.contains(cell)) {
      return;
    }

    const row = +cell.getAttribute("data-row");
    const col = +cell.getAttribute("data-col");

    if (soko.goto(row, col)) {
      renderBoard();

      if (soko.solved()) {
        const score = localStorage.getItem(levelNumber+1);
        if (score) {
          if (score > soko.movesSize()){
            updateBestScore(levelNumber+1, soko.movesSize());
          }
        } 
        else if (!score) {
          updateBestScore(levelNumber+1, soko.movesSize());
        }
        renderLevelComplete();
      }
    }
  });

  renderStatus = () => {
    const statusHTML = `
    <div>Level ${soko.levelNumber()+1}</div>
    <button title="Stats" class="collapse"><span class="material-symbols-outlined">expand_circle_down</span></button>
    <div class="stats">
      <div>current:  ${soko.movesSize()}</div>
      <div>best:  ${(localStorage.getItem(soko.levelNumber()+1) == null)  ? 
        "&infin;" : (localStorage.getItem(soko.levelNumber()+1))} </div>
      <div id="status-box">${soko.sequence()} </div>
    </div
    `;
    document.querySelector("#status").innerHTML = statusHTML;

    const collapseEl = document.querySelector(".collapse");
    const statsEl = document.querySelector(".stats");
    collapseEl.addEventListener("click", event => {
      statsEl.classList.toggle("active");
      if (statsEl.style.maxHeight) {
        statsEl.style.maxHeight = null;
      } 
      else {
        statsEl.style.maxHeight = statsEl.scrollHeight + "px";
      } 
    });
  };

  undoEl.addEventListener("click", event => {
    if (soko.undo()) {
      renderBoard();
    }
  });
  resetEl.addEventListener("click", event => {
    soko.reset();
    renderBoard();
  });
 
  const moves = {
    ArrowLeft: "L",
    ArrowUp: "U",
    ArrowRight: "R",
    ArrowDown: "D",
  };
  document.onkeydown = event => {
    if (event.code in moves) {
      event.preventDefault();

      if (soko.move(moves[event.code])) {
        renderBoard();

        if (soko.solved()) {
          const score = localStorage.getItem(levelNumber+1);
          if (score) {
            if (score > soko.movesSize()){
              updateBestScore(levelNumber+1, soko.movesSize());
            }
          } 
          else if (!score) {
            updateBestScore(levelNumber+1, soko.movesSize());
          }
          renderLevelComplete();
        }
      }
    }
    else if (event.code === "KeyZ" && soko.undo()) {
      renderBoard();
    }
  };

  renderBoard();
};

var Module = {
  preRun: [],
  postRun: [onLoaded],
  print: text => console.log(text),
  printErr: text => console.error(text),
};