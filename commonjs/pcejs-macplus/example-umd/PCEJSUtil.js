(function(f){if(typeof exports==="object"&&typeof module!=="undefined"){module.exports=f()}else if(typeof define==="function"&&define.amd){define([],f)}else{var g;if(typeof window!=="undefined"){g=window}else if(typeof global!=="undefined"){g=global}else if(typeof self!=="undefined"){g=self}else{g=this}g.PCEJSUtil = f()}})(function(){var define,module,exports;return (function(){function r(e,n,t){function o(i,f){if(!n[i]){if(!e[i]){var c="function"==typeof require&&require;if(!f&&c)return c(i,!0);if(u)return u(i,!0);var a=new Error("Cannot find module '"+i+"'");throw a.code="MODULE_NOT_FOUND",a}var p=n[i]={exports:{}};e[i][0].call(p.exports,function(r){var n=e[i][1][r];return o(n||r)},p,p.exports,r,e,n,t)}return n[i].exports}for(var u="function"==typeof require&&require,i=0;i<t.length;i++)o(t[i]);return o}return r})()({1:[function(require,module,exports){
module.exports = {
  loadingStatus: loadingStatus,
}

function setElAttrs(el, attrs) {
  Object.keys(attrs).forEach(function(key) {
    el.setAttribute(key, attrs[key])
  })
  return el
}

function emptyEl(el) {
  while (el.firstChild) el.removeChild(el.firstChild)
  return el
}

function loadingStatus(loadingStatusEl) {
  var initialStatus = loadingStatusEl.innerText
  
  emptyEl(loadingStatusEl)

  var statusEl = setElAttrs(document.createElement("div"), {innerHTML: initialStatus})

  var progressEl = setElAttrs(document.createElement("progress"), {
    value: 0,
    max: 100,
    hidden: true,
  })

  progressEl.style.display = 'inline'

  loadingStatusEl.appendChild(statusEl)
  loadingStatusEl.appendChild(progressEl)

  return  {
    totalDependencies: 0,
    update: function(remainingDependencies) {
      this.totalDependencies = Math.max(this.totalDependencies, remainingDependencies)
      this.setStatus(remainingDependencies)    
    },

    setStatus: function (remainingDependencies) {
      if (this.setStatus.interval) clearInterval(this.setStatus.interval)

      var loadedDependiences = this.totalDependencies - remainingDependencies

      if (remainingDependencies) {
        statusEl.innerHTML = 'Loading... ('+loadedDependiences+'/'+this.totalDependencies+')'
        setElAttrs(progressEl, {
          value: loadedDependiences*100,
          max: this.totalDependencies*100,
          hidden: false,
        })
      } else {
        // close progress element
        setElAttrs(progressEl, {
          value: 0,
          max: 0,
          hidden: true,
        })
        loadingStatusEl.style.display = 'none'
      }
    },
  }
}


},{}]},{},[1])(1)
});
