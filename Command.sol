// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract Command {
    string public latestCommand;
    string public latestProofHash;
    address public owner;
    
    event CommandReceived(string command, uint256 timestamp);
    event ProofSubmitted(string proofHash, uint256 timestamp);
    
    constructor() {
        owner = msg.sender;
    }
    
    function setCommand(string memory newCommand) public {
        latestCommand = newCommand;
        emit CommandReceived(newCommand, block.timestamp);
    }
    
    function submitProof(string memory newProofHash) public {
        latestProofHash = newProofHash;
        emit ProofSubmitted(newProofHash, block.timestamp);
    }
    
    function getStatus() public view returns (string memory command, string memory proof) {
        return (latestCommand, latestProofHash);
    }
}
